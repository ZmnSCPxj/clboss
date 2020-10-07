#ifndef BOSS_MOD_REGULARACTIVEPROBE_HPP
#define BOSS_MOD_REGULARACTIVEPROBE_HPP

namespace S { class Bus; }

namespace Boss { namespace Mod {

class RegularActiveProbe {
private:
	S::Bus& bus;

	void start();

public:
	RegularActiveProbe() =delete;
	RegularActiveProbe(RegularActiveProbe&&) =delete;
	RegularActiveProbe(RegularActiveProbe const&) =delete;

	explicit
	RegularActiveProbe(S::Bus& bus_) : bus(bus_) { start(); }
};

}}

#endif /* !defined(BOSS_MOD_REGULARACTIVEPROBE_HPP) */
