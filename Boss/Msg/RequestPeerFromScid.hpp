#ifndef BOSS_MSG_REQUESTPEERFROMSCID_HPP
#define BOSS_MSG_REQUESTPEERFROMSCID_HPP

#include"Ln/Scid.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestPeerFromScid
 *
 * @brief requests the peer `Ln::NodeId` from the
 * given `Ln::Scid`.
 */
struct RequestPeerFromScid {
	void* requester;
	Ln::Scid scid;
};

}}

#endif /* !defined(BOSS_MSG_REQUESTPEERFROMSCID_HPP) */
