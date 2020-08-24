#undef NDEBUG
#include<Ev/Io.hpp>
#include<Ev/start.hpp>
#include<Ev/yield.hpp>
#include<S/Bus.hpp>
#include<assert.h>
#include<memory>

Ev::Io<void> io_main() {
	auto bus = std::make_shared<S::Bus>();
	auto flag = std::make_shared<bool>();
	auto flag2 = std::make_shared<bool>();
	return Ev::yield().then([=]() {
		/* Should be benign.  */
		return bus->raise(42);
	}).then([=]() {
		*flag = false;
		bus->subscribe<int>([=](int const& i) {
			if (i == 42)
				*flag = true;
			return Ev::lift();
		});
		return bus->raise(40).then([=]() {
			/* Previous subscription should not have
			 * an effect.  */
			assert(!*flag);
			return bus->raise(42);
		}).then([=] () {
			/* Subscription should have triggered.  */
			assert(*flag);
			return Ev::yield();
		});
	}).then([=]() {
		/* Types are strict.  */
		*flag = false;
		return bus->raise<double>(42.0).then([=]() {
			/* Previous subscription should not trigger,
			 * it is registered for int, not double.  */
			assert(!*flag);
			return Ev::lift();
		});
	}).then([=]() {
		*flag = false;
		bus->subscribe<double>([=](double const& i) {
			if (i == 42.0)
				*flag = true;
			return Ev::lift();
		});
		return bus->raise<double>(42.0).then([=]() {
			assert(*flag);
			return Ev::lift();
		});
	}).then([=]() {
		*flag = false;
		*flag2 = false;
		/* Test multiple subscriptions.  */
		bus->subscribe<double>([=](double const& i) {
			*flag2 = true;
			return Ev::lift();
		});
		return bus->raise<double>(42.0).then([=]() {
			/* Both subscriptions should have triggered.  */
			assert(*flag);
			assert(*flag2);
			return Ev::lift();
		});
	});
}

int main() {
	return Ev::start(io_main().then([](){
		return Ev::lift(0);
	}));
}
