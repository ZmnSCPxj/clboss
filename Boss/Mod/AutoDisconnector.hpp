#ifndef BOSS_MOD_AUTODISCONNECTOR_HPP
#define BOSS_MOD_AUTODISCONNECTOR_HPP

namespace Boss { namespace Mod { class Rpc; }}
namespace Boss { namespace Msg { struct Init; }}
namespace Boss { namespace Msg { struct ListpeersAnalyzedResult; }}
namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::AutoDisconnector
 *
 * @brief disconnects from peers if we are connected
 * to too many that do not have channels to us anyway.
 * This limits the number of connections we have.
 */
class AutoDisconnector {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;

	void start();
	Ev::Io<void> on_init(Msg::Init const&);
	Ev::Io<void> on_listpeers(Msg::ListpeersAnalyzedResult const&);

public:
	AutoDisconnector() =delete;
	AutoDisconnector(AutoDisconnector const&) =delete;
	AutoDisconnector(AutoDisconnector&&) =delete;

	AutoDisconnector(S::Bus& bus_
			) : bus(bus_)
			  , rpc(nullptr)
			  { start(); }
};

}}

#endif /* !defined(BOSS_MOD_AUTODISCONNECTOR_HPP) */
