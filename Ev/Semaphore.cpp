#include"Ev/Io.hpp"
#include"Ev/Semaphore.hpp"
#include"Ev/yield.hpp"
#include"Util/make_unique.hpp"
#include<assert.h>
#include<functional>
#include<queue>
#include<utility>

namespace Ev {

class Semaphore::Impl {
private:
	std::size_t remaining;
	/* A currently-pending process.  */
	struct Process {
		Ev::Io<void> action;
		std::function<void()> pass;
		std::function<void(std::exception_ptr)> fail;
		Process( Ev::Io<void> action_
		       , std::function<void()> pass_
		       , std::function<void(std::exception_ptr)> fail_
		       ) : action(std::move(action_))
			 , pass(std::move(pass_))
			 , fail(std::move(fail_))
			 { }
	};
	std::queue<Process> blocked;

	void unblock() {
		if (!blocked.empty()) {
			assert(remaining == 0);
			auto& next = blocked.front();
			auto action = std::move(next.action);
			auto pass = std::move(next.pass);
			auto fail = std::move(next.fail);
			blocked.pop();
			handle( std::move(action)
			      , std::move(pass)
			      , std::move(fail)
			      );
		} else {
			++remaining;
		}
	}
	void handle( Ev::Io<void> action
		   , std::function<void()> pass
		   , std::function<void(std::exception_ptr)> fail
		   ) {
		/* Avoid copying functions, just move them.  */
		auto ppass = std::make_shared<std::function<void()>>(std::move(pass));
		auto pfail = std::make_shared<std::function<void(std::exception_ptr)>>(std::move(fail));
		action.run([ppass, this]() {
			unblock();
			std::move(*ppass)();
		}, [pfail, this](std::exception_ptr e) {
			unblock();
			std::move(*pfail)(e);
		});
	}

public:
	Impl(std::size_t max) : remaining(max), blocked() { }
	Impl() =delete;
	Impl(Impl const&) =delete;
	Impl(Impl&&) =delete;

	Ev::Io<void> run(Ev::Io<void> action_) {
		auto action = std::make_shared<Ev::Io<void>>(std::move(action_));
		return Ev::Io<void>([ this, action
				    ]( std::function<void()> pass
				     , std::function<void(std::exception_ptr)> fail
				     ) {
			if (remaining > 0) {
				--remaining;
				handle( std::move(*action)
				      , std::move(pass)
				      , std::move(fail)
				      );
			} else {
				blocked.emplace( std::move(*action)
					       , std::move(pass)
					       , std::move(fail)
					       );
			}
		}) + Ev::yield();
	}

};

Semaphore::Semaphore(Semaphore&&) =default;
Semaphore::~Semaphore() =default;

Semaphore::Semaphore(std::size_t max)
	: pimpl(Util::make_unique<Impl>(max)) { }

Ev::Io<void> Semaphore::core_run(Ev::Io<void> action) {
	return pimpl->run(Ev::yield() + std::move(action));
}

}
