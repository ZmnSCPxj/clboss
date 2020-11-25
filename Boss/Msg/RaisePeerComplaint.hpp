#ifndef BOSS_MSG_RAISEPEERCOMPLAINT_HPP
#define BOSS_MSG_RAISEPEERCOMPLAINT_HPP

#include"Ln/NodeId.hpp"
#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::RaisePeerComplaint
 *
 * @brief Emitted by modules to raise complaints about bad peers.
 * This should be emitted in direct reaction to the
 * `Boss::Msg::SolicitPeerComplaints` message.
 */
struct RaisePeerComplaint {
	/* The peer we are complaining about.  */
	Ln::NodeId peer;
	/* Human-readable string describing the complaint.  */
	std::string complaint;
};

}}

#endif /* !defined(BOSS_MSG_RAISEPEERCOMPLAINT_HPP) */
