#ifndef EV_THREADPOOL_HPP
#define EV_THREADPOOL_HPP

#include<functional>
#include<memory>
#include"Ev/Io.hpp"

namespace Ev {

/* This thread pool is not to ditribute compute
 * resources;
 * Instead, it is intended to keep the main
 * thread responsive to module activity.
 * Some operations are inherently blocking,
 * especially disk operations, so we put those
 * operations in a background thread and then
 * let the ongoing concurrent task be
 * suspended until awoken by this thread pool.
 * This becomes less important as SSD use goes
 * up.
 */
class ThreadPool {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

	/* i.e. this accepts a function of type
	 * () -> () -> ()
	 * The first stage is executed in a
	 * background thread.
	 * The second stage is executed in
	 * the main thread.
	 */
	void add(std::function<std::function<void()>()>);

public:
	ThreadPool();
	~ThreadPool();
	ThreadPool(ThreadPool const&) =delete;
	ThreadPool(ThreadPool&&) =delete;

	template<typename a>
	Ev::Io<a> background(std::function<a()> func) {
		auto funptr = std::make_shared<std::function<a()>>
			( std::move(func) );
		return Ev::Io<a>([ funptr
				 , this
				 ]( std::function<void(a)> pass
				  , std::function<void(std::exception_ptr)> fail
				  ) {
			auto stage1 = [funptr, pass, fail]() {
				auto stage2 = std::function<void()>();
				try {
					auto res = std::make_shared<a>(
						(*funptr)()
					);
					stage2 = [pass, res]() {
						pass(std::move(*res));
					};
				} catch (...) {
					auto e = std::current_exception();
					stage2 = [fail, e]() {
						fail(e);
					};
				}
				return stage2;
			};
			add(stage1);
		});
	}
};

}

#endif /* EV_THREADPOOL_HPP */
