#include"Boss/Msg/JsonCout.hpp"
#include"Boss/log.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<stdarg.h>

namespace Boss {

Ev::Io<void> log(S::Bus& bus, LogLevel l, const char *fmt, ...) {
	va_list ap;

	auto written = size_t(0);
	auto size = size_t(42);
	auto buf = std::unique_ptr<char[]>();
	do {
		if (size <= written)
			size = written + 1;
		buf = Util::make_unique<char[]>(size);
		va_start(ap, fmt);
		written = size_t(vsnprintf(buf.get(), size, fmt, ap));
		va_end(ap);
	} while (size <= written);

	auto msg = std::string(buf.get());

	auto level_string = std::string();
	switch (l) {
	case Debug: level_string = std::string("debug"); break;
	case Info: level_string = std::string("info"); break;
	case Warn: level_string = std::string("warn"); break;
	case Error: level_string = std::string("error"); break;
	}

	auto js = Json::Out()
		.start_object()
			.field("jsonrpc", std::string("2.0"))
			.field("method", std::string("log"))
			.start_object("params")
				.field("level", level_string)
				.field("message", msg)
			.end_object()
		.end_object()
		;

	/* TODO: Instead of raising JsonCout directly, raise a
	 * Log message and have a module translate that, as well as
	 * possibly save the log for programmatic access.  */

	return bus.raise(Boss::Msg::JsonCout{std::move(js)});
}

}
