#ifndef BOSS_MSG_SOLICITPEERCOMPLAINTS_HPP
#define BOSS_MSG_SOLICITPEERCOMPLAINTS_HPP

#include"Boss/Msg/PeerMetrics.hpp"
#include"Ln/NodeId.hpp"
#include<map>

namespace Boss { namespace Msg {

/** struct Boss::Msg::SolicitPeerComplaints
 *
 * @brief emitted periodically to solicit complaints about bad
 * peers from our modules.
 * Modules should emit `Boss::Msg::RaisePeerComplaint` messages
 * in direct reaction to this message.
 *
 * @desc This message also contains the latest peer metrics, for
 * the convenience of modules which just use the peer metrics.
 */
struct SolicitPeerComplaints {
	std::map<Ln::NodeId, PeerMetrics> day3;
	std::map<Ln::NodeId, PeerMetrics> weeks2;
	std::map<Ln::NodeId, PeerMetrics> month1;
};

}}

#endif /* !defined(BOSS_MSG_SOLICITPEERCOMPLAINTS_HPP) */
