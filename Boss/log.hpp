#ifndef BOSS_LOG_HPP
#define BOSS_LOG_HPP

namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss {

enum LogLevel {
	Debug,
	Info,
	Warn,
	Error
};

Ev::Io<void> log(S::Bus& bus, LogLevel l, const char *fmt, ...);

}

#endif /* BOSS_LOG_HPP */
