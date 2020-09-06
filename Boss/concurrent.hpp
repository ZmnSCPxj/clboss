#ifndef BOSS_CONCURRENT_HPP
#define BOSS_CONCURRENT_HPP

namespace Ev { template<typename a> class Io; }

namespace Boss {

/** Boss::concurrent.
 *
 * @brief Like Ev::concurrent except it ignores
 * Boss::Shutdown exceptions in the new greenthread.
 *
 * @desc Schedules a new greenthread for launching
 * later.
 * Boss::Shutdown exceptions in the new greenthread
 * are ignored.
 */
Ev::Io<void> concurrent(Ev::Io<void>);

}

#endif /* !defined(BOSS_CONCURRENT_HPP) */
