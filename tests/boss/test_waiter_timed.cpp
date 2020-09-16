#undef NDEBUG
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Shutdown.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"
#include"S/Bus.hpp"
#include<assert.h>
#include<memory>

int main() {
	S::Bus bus;
	Boss::Mod::Waiter waiter(bus);

	auto loop_test = [&]() {
		auto counter = std::make_shared<std::size_t>(1000);
		auto ploop = std::make_shared<std::function<Ev::Io<void>()>
					     >();
		*ploop = [&waiter, counter, ploop]() {
			if ((*counter) == 0) {
				*ploop = nullptr;
				return Ev::lift();
			}
			--(*counter);

			return Ev::lift().then([&waiter]() {
				/* Yield should complete first.  */
				auto action = Ev::yield().then([]() {
					return Ev::lift(true);
				});
				return waiter.timed(60, action
						   ).catching<Boss::Mod::Waiter::TimedOut>([](Boss::Mod::Waiter::TimedOut const& _) {
					return Ev::lift(false);
				});
			}).then([ploop](bool flag) {
				assert(flag);
				return (*ploop)();
			});
		};

		return (*ploop)();
	};

	auto code = Ev::lift().then([&]() {

		/* Trivial timed.  */
		return waiter.timed(60, Ev::lift(42));
	}).then([&](int i) {
		assert(i == 42);

		/* Core operation delays, but completes first.  */
		return waiter.timed(60, waiter.wait(0.001));
	}).then([&]() {

		/* Timeout reached first.  */
		return waiter.timed(0.001, Ev::lift().then([&]() {
			return waiter.wait(60);
		}).then([]() {
			return Ev::lift(true);
		})).catching<Boss::Mod::Waiter::TimedOut>([](Boss::Mod::Waiter::TimedOut const& _) {
			return Ev::lift(false);
		});
	}).then([&](bool flag) {
		assert(!flag);

		return loop_test();
	}).then([&]() {

		/* Cancel all pending waiters.  */
		return bus.raise(Boss::Shutdown());
	}).then([]() {
		return Ev::lift(0);
	});

	return Ev::start(code);
}
