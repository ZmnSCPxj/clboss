#ifndef EV_YIELD_HPP
#define EV_YIELD_HPP

#include<cstddef>

namespace Ev { template<typename a> class Io; }

namespace Ev {

/** Ev::yield
 *
 * @brief Does nothing, but allows other Ev::Io greenthreads
 * to continue processing.
 *
 * @desc A yield point, where other concurrent tasks may execute.
 *
 * Recommended to put at each loop.
 *
 * Because other concurrent tasks may execute while your task
 * goes through this function, you should be aware that variables
 * you are using may change.
 * This warning applies to all yield points, and other functions
 * should mention whether they are yield points and have this
 * property as well.
 *
 * @param num_yields - How many times to yield.
 * Usually omitted (meaning just yield once); this is generally
 * only used in testcases to allow modules to operate.
 */
Ev::Io<void> yield();

Ev::Io<void> yield(std::size_t num_yields);

}

#endif /* !defined(EV_YIELD_HPP) */
