#include"Boss/Msg/JsonCout.hpp"
#include"Boss/log.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include"Util/Str.hpp"
#include<stdarg.h>

namespace Boss {

Ev::Io<void> log(S::Bus& bus, LogLevel l, const char *fmt, ...) {
	va_list ap;

	auto msg = std::string();

	va_start(ap, fmt);
	msg = Util::Str::vfmt(fmt, ap);
	va_end(ap);

	auto level_string = std::string();
	switch (l) {
	case Trace: level_string = std::string("trace"); break;
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
