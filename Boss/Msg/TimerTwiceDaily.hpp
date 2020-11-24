#ifndef BOSS_MSG_TIMERTWICEDAILY_HPP
#define BOSS_MSG_TIMERTWICEDAILY_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::TimerTwiceDaily
 *
 * @brief emitted about twice a day.
 * If more than half a day has gone since the
 * previous emission, emitted asynchronously
 * at `Init`.
 */
struct TimerTwiceDaily {};

}}

#endif /* !defined(BOSS_MSG_TIMERTWICEDAILY_HPP) */
