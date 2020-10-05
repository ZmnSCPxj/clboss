#ifndef BOSS_MOD_FORWARDFEEMONITOR_HPP
#define BOSS_MOD_FORWARDFEEMONITOR_HPP

#include"Ln/NodeId.hpp"
#include"Ln/Scid.hpp"
#include<map>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ForwardFeeMonitor
 *
 * @brief Monitors `forward_event`s and broadcasts
 * `Boss::Msg::ForwardFee`.
 */
class ForwardFeeMonitor {
private:
	S::Bus& bus;
	/* Mapping from SCIDs to the peer IDs that have those channels.  */
	std::map<Ln::Scid, Ln::NodeId> peers;

	void start();

public:
	ForwardFeeMonitor() =delete;
	ForwardFeeMonitor(ForwardFeeMonitor&&) =delete;
	ForwardFeeMonitor(ForwardFeeMonitor const&) =delete;

	explicit
	ForwardFeeMonitor(S::Bus& bus_) : bus(bus_) { start(); }
};

}}

#endif /* !defined(BOSS_MOD_FORWARDFEEMONITOR_HPP) */
