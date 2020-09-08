#include"Boss/Mod/Timers.hpp"
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/Timer10Minutes.hpp"
#include"Boss/Msg/TimerRandomDaily.hpp"
#include"Boss/Msg/TimerRandomHourly.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"S/Bus.hpp"
#include<chrono>
#include<memory>
#include<random>

namespace {

template<typename T>
class RandomTimeLoop : public std::enable_shared_from_this<RandomTimeLoop<T>> {
private:
	std::default_random_engine engine;
	std::string name;
	S::Bus& bus;
	Boss::Mod::Waiter& waiter;
	std::uniform_real_distribution<double> dist;

	/* FIXME: Use a better seed.  */
	RandomTimeLoop( std::string name_
		      , S::Bus& bus_
		      , Boss::Mod::Waiter& waiter_
		      , std::uniform_real_distribution<double> dist_
		      ) : engine(std::chrono::system_clock::now().time_since_epoch().count())
			, name(std::move(name_))
			, bus(bus_)
			, waiter(waiter_)
			, dist(std::move(dist_)) {
	}

	Ev::Io<void> loop() {
		return Ev::lift().then([this]() {
			auto time = dist(engine);
			return waiter.wait(time);
		}).then([this]() {
			return Boss::log( bus, Boss::Debug
					, "Timers: triggering %s"
					, name.c_str()
					);
		}).then([this]() {
			return Boss::concurrent(bus.raise(T()));
		}).then([this]() {
			return loop();
		});
	}

public:
	~RandomTimeLoop() { }
	static
	std::shared_ptr<RandomTimeLoop>
	create( std::string name
	      , S::Bus& bus
	      , Boss::Mod::Waiter& waiter
	      , std::uniform_real_distribution<double> dist
	      ) {
		/* Cannot use std::make_shared, constructor is private.  */
		return std::shared_ptr<RandomTimeLoop>(
			new RandomTimeLoop( std::move(name)
					  , bus
					  , waiter
					  , std::move(dist)
					  )
		);
	}

	Ev::Io<void> enter_loop() {
		auto self = RandomTimeLoop<T>::shared_from_this();
		return self->loop().then([self]() {
			/* This function becomes part of the
			 * pass continuation that is given to
			 * the loop operation.
			 * This keeps the self-pointer alive.
			 * This function never actually gets
			 * invoked, but if an exception is
			 * thrown within the Ev::Io system,
			 * this function get destructed, and
			 * only then will the self-pointer get
			 * destructed.
			 */
			return Ev::lift();
		});
	}
};

template<typename T>
Ev::Io<void> random_time_loop( std::string name
			     , S::Bus& bus
			     , Boss::Mod::Waiter& waiter
			     , std::uniform_real_distribution<double> dist
			     ) {
	auto loop_obj = RandomTimeLoop<T>::create(
		std::move(name), bus, waiter, std::move(dist)
	);
	return loop_obj->enter_loop();
}

}

namespace Boss { namespace Mod {

void Timers::start() {
	bus.subscribe<Boss::Msg::Init>([this](Boss::Msg::Init const& _) {
		/* Start timers at init.  */
		return Ev::lift().then([this]() {
			return Boss::concurrent(random_daily());
		}).then([this]() {
			return Boss::concurrent(random_hourly());
		}).then([this]() {
			return Boss::concurrent(every_10_minutes());
		});
	});
}

Ev::Io<void> Timers::random_daily() {
	auto dist = std::uniform_real_distribution<double>(43200, 86400 + 43200);
	return random_time_loop<Boss::Msg::TimerRandomDaily>(
		"random daily",
		bus, waiter, std::move(dist)
	);
}
Ev::Io<void> Timers::random_hourly() {
	auto dist = std::uniform_real_distribution<double>(1800, 3600 + 1800);
	return random_time_loop<Boss::Msg::TimerRandomHourly>(
		"random hourly",
		bus, waiter, std::move(dist)
	);
}
Ev::Io<void> Timers::every_10_minutes() {
	/* Reuse the time loop to reduce our code burden.  */
	auto dist = std::uniform_real_distribution<double>(600, 600);
	return random_time_loop<Boss::Msg::Timer10Minutes>(
		"regular 10 minutes",
		bus, waiter, std::move(dist)
	);
}

}}
