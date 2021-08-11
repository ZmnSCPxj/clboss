#include"Util/format.hpp"
#include"Util/make_unique.hpp"
#include<memory>
#include<stdlib.h>

namespace Util {

std::string vformat(char const* fmt, va_list ap_orig) {
	va_list ap;

	auto written = size_t(0);
	auto size = size_t(42);
	auto buf = std::unique_ptr<char[]>();
	do {
		if (size <= written)
			size = written + 1;
		buf = Util::make_unique<char[]>(size);
		va_copy(ap, ap_orig);
		written = size_t(vsnprintf(buf.get(), size, fmt, ap));
		va_end(ap);
	} while (size <= written);

	return std::string(buf.get());
}
std::string format(char const* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	auto rv = vformat(fmt, ap);
	va_end(ap);
	return rv;
}

}
