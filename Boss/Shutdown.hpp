#ifndef BOSS_SHUTDOWN_HPP
#define BOSS_SHUTDOWN_HPP

namespace Boss {

/** struct Boss::Shutdown
 *
 * @brief exception thrown by blocking Ev::Io
 * operations when the operator is being
 * destructed.
 */
struct Shutdown {};

}

#endif /* !defined(BOSS_SHUTDOWN_HPP) */
