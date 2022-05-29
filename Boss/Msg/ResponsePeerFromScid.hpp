#ifndef BOSS_MSG_RSEPONSEPEERFROMSCID_HPP
#define BOSS_MSG_RSEPONSEPEERFROMSCID_HPP

#include"Ln/NodeId.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponsePeerFromScid
 *
 * @brief returns the peer `Ln::NodeId` from the given
 * `Ln::Scid`.
 * Broadcast in response to `Boss::Msg::RequestPeerFromScid`.
 * The peer may be `nullptr` if the given `Ln::Scid` was
 * not recognized.
 */
struct ResponsePeerFromScid {
	void* requester;
	Ln::Scid scid;
	Ln::NodeId peer;
};

}}

#endif /* !defined(BOSS_MSG_RSEPONSEPEERFROMSCID_HPP) */
