#ifndef BOSS_MOD_TIMERS_HPP
#define BOSS_MOD_TIMERS_HPP

namespace Boss { namespace Mod { class Waiter; }}
namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::Timers
 *
 * @brief emits a variety of timing messages.
 *
 * @desc Emits timing messages:
 *
 * - Boss::Msg::TimerRandomDaily
 *   Approximately once a day but with some
 *   randomness.
 * - Boss::Msg::TimerRandomHourly
 *   Approximately once an hour but with
 *   some randomness.
 * - Boss::Msg::Timer10Minutes
 *   Every 10 minutes.
 */
class Timers {
private:
	S::Bus& bus;
	Boss::Mod::Waiter& waiter;

	Ev::Io<void> random_daily();
	Ev::Io<void> random_hourly();
	Ev::Io<void> every_10_minutes();

	void start();

public:
	Timers( S::Bus& bus_
	      , Boss::Mod::Waiter& waiter_
	      ) : bus(bus_)
		, waiter(waiter_)
		{
		start();
	}
};

}}

#endif /* BOSS_MOD_TIMERS_HPP */
