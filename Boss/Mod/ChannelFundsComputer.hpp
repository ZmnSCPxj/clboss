#ifndef BOSS_MOD_CHANNELFUNDSCOMPUTER_HPP
#define BOSS_MOD_CHANNELFUNDSCOMPUTER_HPP

namespace Boss { namespace Msg { class ListfundsResult; }}
namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

class ChannelFundsComputer {
private:
	S::Bus& bus;

	void start();
	Ev::Io<void> on_listfunds(Msg::ListfundsResult const&);

public:
	ChannelFundsComputer() =delete;
	ChannelFundsComputer(ChannelFundsComputer const&) =delete;
	ChannelFundsComputer(ChannelFundsComputer&&) =delete;

	explicit
	ChannelFundsComputer(S::Bus& bus_) : bus(bus_) { start(); }
};

}}

#endif /* !defined(BOSS_MOD_CHANNELFUNDSCOMPUTER_HPP) */
