#ifndef BOSS_LOG_HPP
#define BOSS_LOG_HPP

#ifdef HAVE_CONFIG_H
# include"config.h"
#endif

namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss {

enum LogLevel {
	Trace,
	Debug,
	Info,
	Warn,
	Error
};

Ev::Io<void> log(S::Bus& bus, LogLevel l, const char *fmt, ...)
#if HAVE_ATTRIBUTE_FORMAT
	__attribute__ ((format (printf, 3, 4)))
#endif
;

}

#endif /* BOSS_LOG_HPP */
