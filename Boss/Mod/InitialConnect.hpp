#ifndef BOSS_MOD_INITIALCONNECT_HPP
#define BOSS_MOD_INITIALCONNECT_HPP

#include<string>

namespace Boss { namespace Msg { struct ListpeersAnalyzedResult; }}
namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class InitialConnect
 *
 * @brief determines if we need connections, and emits
 * Boss::Msg::NeedsConnect if so.
 *
 * @desc a new LN node will have no connections.
 * This handles the initial connection logic.
 */
class InitialConnect {
private:
	S::Bus& bus;

	Ev::Io<void> needs_connect(std::string reason);
	Ev::Io<void> init_check(Msg::ListpeersAnalyzedResult const&);
	Ev::Io<void> periodic_check(Msg::ListpeersAnalyzedResult const&);

	void start();

public:
	explicit
	InitialConnect(S::Bus& bus_) : bus(bus_) { start(); }
	InitialConnect() =delete;
	InitialConnect(InitialConnect const&) =delete;
};

}}

#endif /* !defined(BOSS_MOD_INITIALCONNECT_HPP) */
