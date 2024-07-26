#include<assert.h>
#include<condition_variable>
#include<errno.h>
#include<ev.h>
#include<fcntl.h>
#include<mutex>
#include<queue>
#include<signal.h>
#include<stdexcept>
#include<string.h>
#include<thread>
#include<unistd.h>
#include<vector>
#include"Ev/ThreadPool.hpp"
#include"Util/BacktraceException.hpp"
#include"Util/make_unique.hpp"

namespace {

/* RAII class to block all signals while still alive.  */
class SigBlocker {
private:
	sigset_t old_set;
public:
	SigBlocker(SigBlocker&&) =delete;
	SigBlocker(SigBlocker const&) =delete;

	SigBlocker() {
		sigset_t blockall;
		sigfillset(&blockall);
		pthread_sigmask(SIG_SETMASK, &blockall, &old_set);
	}
	~SigBlocker() {
		pthread_sigmask(SIG_SETMASK, &old_set, nullptr);
	}
};

}

namespace Ev {

class ThreadPool::Impl {
private:
	/*-------------------------------------------------------*/
	/* These objects are handled only by the
	 * main thread.
	 */
	/* Background threads.  */
	std::vector<std::thread> threads;

	/* Number of pending added tasks.  */
	std::size_t num_tasks;

	/* An ev_io watcher that is waiting for
	 * the read pipe.
	 */
	std::unique_ptr<ev_io> io_waiter;

	/*-------------------------------------------------------*/
	/* This is only set during initialization.  */
	int pipe_write;
	int pipe_read;

	/*-------------------------------------------------------*/
	/* These objects are shared across threads.
	 * Hold the mutex while using these!
	 */
	std::mutex mtx;

	/* Used to wake up a background thread if
	 * there is work to be done.
	 */
	std::condition_variable cnd;

	/* Set to true if this is being destructed.  */
	bool shutdown;

	/* Queue of commands to execute in the
	 * background thread.
	 */
	std::queue<std::function<std::function<void()>()>>
		work_queue;

	/* Queue of results to execute in the
	 * main thread.
	 */
	std::queue<std::function<void()>> result_queue;

private:
	/* Gets from the work queue.
	 * Precondition: the mutex must be locked.
	 */
	std::function<std::function<void()>()>
	get_work(std::unique_lock<std::mutex>& locker) {
		while (work_queue.empty() && !shutdown)
			cnd.wait(locker);
		if (shutdown)
			return nullptr;
		auto ret = std::move(work_queue.front());
		work_queue.pop();
		return ret;
	}

	/* Puts to the result queue.
	 * Precondition: the mutex must be locked.
	 */
	void put_result(std::function<void()> result) {
		result_queue.emplace(std::move(result));
		/* Wake up main thread.  */
		auto c = char(1);
		auto res = ssize_t();
		do {
			res = write(pipe_write, &c, 1);
		} while (res < 0 && errno == EINTR);
	}

	/* Run at each backgrond thread.  */
	void background() {
		auto locker = std::unique_lock<std::mutex>(mtx);
		for (;;) {
			auto work = get_work(locker);
			locker.unlock();

			if (!work)
				return;

			auto result = work();
			work = nullptr;

			locker.lock();
			put_result(std::move(result));
		}
	}

	/* Run to handle an ev_io event.  */
	void io_handler(EV_P) {
		/* Remove one notification from the pipe.
		 * FIXME
		 * It would theoretically be better to just
		 * read as much as is available until EAGAIN
		 * or EWOULDBLOCK, and then pull that many
		 * from the queue, but that optimization can
		 * be done later.
		 */
		auto c = char();
		auto res = ssize_t();
		do {
			res = read(pipe_read, &c, 1);
		} while (res < 0 && errno == EINTR);

		/* Grab one.  */
		auto result = ([this](){
			auto locker = std::unique_lock<std::mutex>(mtx);
			auto ret = std::move(result_queue.front());
			result_queue.pop();
			return ret;
		})();

		--num_tasks;
		if (num_tasks == 0) {
			/* No more tasks, so we can stop the watcher.
			 * This is important as we have the rule that
			 * all watchers must stop on a breaking signal.
			 */
			ev_io_stop(EV_A_ io_waiter.get());
			io_waiter = nullptr;
		}

		result();
	}

	static
	void io_handler_static(EV_P_ ev_io* raw_io_waiter, int revents) {
		auto self = (Impl*)raw_io_waiter->data;
		return self->io_handler(EV_A);
	}

public:
	Impl() {
		num_tasks = 0;
		shutdown = false;

		int pipes[2];
		auto pipe_res = pipe(pipes);
		if (pipe_res < 0) {
			throw Util::BacktraceException<std::runtime_error>(std::string("Ev::ThreadPool: pipe:")
						+ strerror(errno)
						);
		}

		pipe_read = pipes[0];
		pipe_write = pipes[1];
		{
			/* Make read end non-blocking.  */
			auto flags = fcntl(pipe_read, F_GETFL);
			flags |= O_NONBLOCK;
			fcntl(pipe_read, F_SETFL, flags);
		}

		/* Launch threads with all signals blocked.
		 * On exit from this function, the current
		 * thread has its signal mask restored.
		 */
		SigBlocker blocker;
		for (auto i = 0; i < 16; ++i)
			threads.emplace_back([this]() { background(); });
	}

	void add(std::function<std::function<void()>()> work) {
		assert(work);
		{
			auto locker = std::unique_lock<std::mutex>(mtx);
			work_queue.emplace(std::move(work));
		}
		cnd.notify_one();
		++num_tasks;

		if (!io_waiter) {
			io_waiter = Util::make_unique<ev_io>();
			ev_io_init( io_waiter.get()
				  , &io_handler_static
				  , pipe_read
				  , EV_READ
				  );
			io_waiter->data = this;
			ev_io_start(EV_DEFAULT_ io_waiter.get());
		}
	}

	~Impl() {
		if (io_waiter)
			ev_io_stop(EV_DEFAULT_ io_waiter.get());

		/* Signal the shutdown.  */
		{
			auto locker = std::unique_lock<std::mutex>(mtx);
			shutdown = true;
		}
		cnd.notify_all();
		/* Wait for all threads to finish.  */
		for (auto& t : threads)
			t.join();
	}
};

ThreadPool::ThreadPool()
	: pimpl(Util::make_unique<Impl>()) { }
ThreadPool::~ThreadPool() { }

void
ThreadPool::add(std::function<std::function<void()>()> work) {
	return pimpl->add(std::move(work));
}

}

