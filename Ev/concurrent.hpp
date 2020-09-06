#ifndef EV_CONCURRENT_HPP
#define EV_CONCURRENT_HPP

namespace Ev { template<typename a> class Io; }

namespace Ev {

/** Ev::concurrent
 *
 * @brief causes the given IO action to be
 * invoked later when the current greenthread
 * yields.
 * Starts a new greenthread with that IO
 * action.
 */
Ev::Io<void> concurrent(Ev::Io<void> io);

}

#endif /* EV_CONCURRENT_HPP */
