#ifndef UTIL_FORMAT_HPP
#define UTIL_FORMAT_HPP

#ifdef HAVE_CONFIG_H
# include"config.h"
#endif

#include<stdarg.h>
#include<string>

namespace Util {

/** Util::format
 *
 * @brief Like `sprintf`, but returns `std::string`.
 */
std::string format(char const* fmt, ...)
#if HAVE_ATTRIBUTE_FORMAT
	__attribute__ ((format (printf, 1, 2)))
#endif
;
/** Util::vformat
 *
 * @brief Like `Util::format`, but accepts `va_list`.
 */
std::string vformat(char const* fmt, va_list ap);

}

#endif /* UTIL_FORMAT_HPP */
