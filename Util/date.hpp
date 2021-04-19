#ifndef UTIL_DATE_HPP
#define UTIL_DATE_HPP

#include<string>

namespace Util {

/** Util::date
 *
 * @brief Returns a simple date representation
 * of the given Unix Epoch time.
 *
 * @desc This function may only be called from the main thread
 * of the program.
 * If used with `Ev`, do not call this function from a function
 * you pass to `Ev::ThreadPool`.
 */
std::string date(double epoch);

}

#endif
