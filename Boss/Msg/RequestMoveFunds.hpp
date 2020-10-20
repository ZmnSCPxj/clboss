#ifndef BOSS_MSG_REQUESTMOVEFUNDS_HPP
#define BOSS_MSG_REQUESTMOVEFUNDS_HPP

#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestMoveFunds
 *
 * @brief Requests to move funds from the channel of one
 * peer to the channel of another peer.
 */
struct RequestMoveFunds {
	/* Pointer to the object that requested moving
	 * funds.  */
	void* requester;
	/* Node from which channel funds will be removed.  */
	Ln::NodeId source;
	/* Node to which channel funds will be moved.  */
	Ln::NodeId destination;
	/* Amount to move.  */
	Ln::Amount amount;
	/* Fee budget.  */
	Ln::Amount fee_budget;
};

}}

#endif /* !defined(BOSS_MSG_REQUESTMOVEFUNDS_HPP) */
