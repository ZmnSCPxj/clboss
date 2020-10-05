#ifndef BOSS_MSG_FORWARDFEE_HPP 
#define BOSS_MSG_FORWARDFEE_HPP 

#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ForwardFee
 *
 * @brief Informs everyone of a *successful* forwarding
 * performed by us.
 */
struct ForwardFee {
	/* The incoming peer.  */
	Ln::NodeId in_id;
	/* The outgoing peer.  */
	Ln::NodeId out_id;
	/* The fee.  */
	Ln::Amount fee;
	/* The time, in seconds, it took from us receiving the incoming HTLC
	 * to us receiving the preimage from the outgoing HTLC.  */
	double resolution_time;
};

}}

#endif /* !defined(BOSS_MSG_FORWARDFEE_HPP) */
