#include"Boss/Mod/Waiter.hpp"
#include"Boss/Shutdown.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<ev.h>
#include<list>

namespace Boss { namespace Mod {

class Waiter::Impl {
private:
	bool is_shutting_down;

	/* Information structure for each timer.  */
	struct Info {
		Impl *pimpl;
		std::function<void()> pass;
		std::function<void(std::exception_ptr)> fail;
		std::list<ev_timer>::iterator it;
	};
	std::list<ev_timer> timers;

	void shutdown() {
		is_shutting_down = true;
		/* Move the timers out of the current object and into
		 * a scoped variable.  */
		auto timers_copy = std::move(timers);

		for (auto& timer : timers_copy) {
			/* Reacquire control of the info structure.  */
			auto info = std::unique_ptr<Info>((Info*) timer.data);
			/* Get the fail function.  */
			auto fail = std::move(info->fail);
			/* Stop the timer.  */
			ev_timer_stop(EV_DEFAULT_ &timer);

			/* Resume.  */
			fail_shutdown(fail);
		}
		/* timers_copy should be freed here.  */
	}

	void fail_shutdown(std::function<void(std::exception_ptr)> fail) {
		try {
			throw Boss::Shutdown();
		} catch (...) {
			fail(std::current_exception());
		}
	}

	static
	void timer_static_handler(EV_P_ ev_timer *timer, int revents) {
		/* Reacquire control of the info structure.  */
		auto info = std::unique_ptr<Info>((Info*)timer->data);
		/* Get the pass function.  */
		auto pass = std::move(info->pass);
		/* Stop the timer.  */
		ev_timer_stop(EV_A_ timer);
		/* Remove the timer from the list.  */
		info->pimpl->timers.erase(info->it);

		/* Resume.  */
		pass();
	}

public:
	Impl(S::Bus& bus) {
		is_shutting_down = false;
		bus.subscribe<Boss::Shutdown>([this](Boss::Shutdown const& _) {
			shutdown();
			return Ev::lift();
		});
	}

	Ev::Io<void> wait(double seconds) {
		return Ev::Io<void>([ this
				    , seconds
				    ]( std::function<void()> pass
				     , std::function<void(std::exception_ptr)> fail
				     ) {
			if (is_shutting_down)
				return fail_shutdown(fail);

			/* Create the timer.  */
			auto it = timers.emplace( timers.begin()
						, ev_timer()
						);
			ev_timer_init(&*it, &timer_static_handler, seconds, 0);
			/* Create the object.  */
			auto info = Util::make_unique<Info>();
			info->pimpl = this;
			info->pass = std::move(pass);
			info->fail = std::move(fail);
			info->it = it;
			/* Release the info to the ev_timer.  */
			it->data = info.release();
			/* Give the timer to C.  */
			ev_timer_start(EV_DEFAULT_ &*it);
		});
	}

	struct TimedCoreData {
		typedef std::function<void()> PassF;
		typedef std::function<void(std::exception_ptr)> FailF;
		PassF pass;
		FailF fail;
		bool flag;
	};

	Ev::Io<void> timed_core(double timeout, Ev::Io<void> action) {
		auto paction = std::make_shared<Ev::Io<void>>(
			std::move(action)
		);
		return Ev::Io<void>([ this
				    , timeout
				    , paction
				    ]( std::function<void()> pass
				     , std::function<void(std::exception_ptr)> fail
				     ) {
			auto sh = std::make_shared<TimedCoreData>(TimedCoreData{
				std::move(pass), std::move(fail), false
			});
			auto sub_pass = [sh]() {
				/* Pass case.  */
				if (sh->flag)
					return;
				sh->flag = true;
				auto pass = std::move(sh->pass);
				sh->fail = nullptr;
				pass();
			};
			auto sub_fail = [sh](std::exception_ptr e) {
				/* Pass case.  */
				if (sh->flag)
					return;
				sh->flag = true;
				auto fail = std::move(sh->fail);
				sh->pass = nullptr;
				fail(e);
			};
			wait(timeout).run([sub_fail]() {
				try {
					throw TimedOut{};
				} catch (...) {
					sub_fail(std::current_exception());
				}
			}, sub_fail);
			paction->run(sub_pass, sub_fail);
		}).then([]() {
			return Ev::yield();
		});
	}
};

Waiter::Waiter(S::Bus& bus) : pimpl(Util::make_unique<Impl>(bus)) {}
Waiter::~Waiter() { }

Ev::Io<void> Waiter::wait(double seconds) {
	return pimpl->wait(seconds);
}
Ev::Io<void> Waiter::timed_core(double timeout, Ev::Io<void> action) {
	return pimpl->timed_core(timeout, std::move(action));
}

}}
