#include"Util/date.hpp"
#include<cstdint>
#include<inttypes.h>
#include<math.h>
#include<stdio.h>
#include<time.h>

#if HAVE_CONFIG_H
#include"config.h"
#endif

#if HAVE_GMTIME_R
#define GMTIME_R gmtime_r
#else /* !HAVE_GMTIME_R */
#define GMTIME_R my_gmtime_r
namespace {

/* Emulate with the non-reentrant gmtime.
 *
 * Since any `Ev::Io` program is single-threaded, we only need to
 * ensure this function is not called from an `Ev::ThreadPool`.
 */
struct tm* my_gmtime_r(const time_t* ptime, struct tm* result) {
	auto staticres = gmtime(ptime);
	*result = *staticres;
	return result;
}

}
#endif

namespace Util {

std::string date(double epoch) {
	auto seconds = time_t(floor(epoch));
	auto split = tm();

	(void) GMTIME_R(&seconds, &split);

	/* Should be big enough.  */
	char buffer[4096];
	buffer[0] = '\0';

	(void) strftime(buffer, sizeof(buffer), "UTC %Y-%m-%d %H:%M:%S", &split);

	/* Milliseconds.  */
	auto subsecond = epoch - double(seconds);
	auto milliseconds = std::uint32_t(floor(subsecond * 1000));
	/* In case of roundoff error... maybe.  */
	if (milliseconds > 999)
		milliseconds = 999;
	/* Find append point.  */
	auto p = buffer;
	for (p = buffer; *p; ++p);
	/* Append.  Should still fit in buffer, but use snprintf
	 * just because sprintf is deprecated.  */
	(void) snprintf(p, buffer + sizeof(buffer) - p, ".%03" PRIu32, milliseconds);

	return std::string(buffer);
}

}
