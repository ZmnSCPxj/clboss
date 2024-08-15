#ifndef BOSS_MOD_FORWARDFEEMONITOR_HPP
#define BOSS_MOD_FORWARDFEEMONITOR_HPP

#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/RequestPeerFromScid.hpp"
#include"Boss/Msg/ResponsePeerFromScid.hpp"

namespace Ln { class Amount; }
namespace Ln { class NodeId; }
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

	Boss::ModG::ReqResp< Msg::RequestPeerFromScid
			   , Msg::ResponsePeerFromScid
			   > peer_from_scid_rr;

	void start();
	Ev::Io<void> cont( Ln::NodeId in_id
			 , Ln::NodeId out_id
			 , Ln::Amount fee
			 , double resolution_time
			 , Ln::Amount amount
			 );

public:
	ForwardFeeMonitor() =delete;
	ForwardFeeMonitor(ForwardFeeMonitor&&) =delete;
	ForwardFeeMonitor(ForwardFeeMonitor const&) =delete;

	explicit
	ForwardFeeMonitor(S::Bus& bus_
			 ) : bus(bus_)
			   , peer_from_scid_rr(bus_)
			   { start(); }
};

}}

#endif /* !defined(BOSS_MOD_FORWARDFEEMONITOR_HPP) */
