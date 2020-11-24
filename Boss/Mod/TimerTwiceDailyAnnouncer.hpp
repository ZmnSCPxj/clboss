#ifndef BOSS_MOD_TIMERTWICEDAILYANNOUNCER_HPP
#define BOSS_MOD_TIMERTWICEDAILYANNOUNCER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::TimerTwiceDailyAnnouncer
 *
 * @brief Announces `Msg::TimerTwiceDaily` at slightly less
 * than twice a day.
 * Ensures that if the previous announce was more than half
 * day ago (e.g. we were down) it gets announced now.
 */
class TimerTwiceDailyAnnouncer {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	TimerTwiceDailyAnnouncer() =delete;
	TimerTwiceDailyAnnouncer(TimerTwiceDailyAnnouncer const&) =delete;

	TimerTwiceDailyAnnouncer(TimerTwiceDailyAnnouncer&&);
	~TimerTwiceDailyAnnouncer();
	explicit
	TimerTwiceDailyAnnouncer(S::Bus&);
};

}}

#endif /* !defined(BOSS_MOD_TIMERTWICEDAILYANNOUNCER_HPP) */
