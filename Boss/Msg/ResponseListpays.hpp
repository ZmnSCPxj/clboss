#ifndef BOSS_MSG_RESPONSELISTPAYS_HPP
#define BOSS_MSG_RESPONSELISTPAYS_HPP

#include"Boss/Msg/StatusListpays.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include"Sha256/Hash.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponseListpays
 *
 * @brief emitted in response to a
 * `Boss::Msg::RequestListpays` message.
 * Contains details of the payment.
 */
struct ResponseListpays {
	Sha256::Hash payment_hash;

	/* Status of the payment.  */
	StatusListpays status;
	/* Who the payee is.
	 * Not valid if status is nonexistent.
	 */
	Ln::NodeId payee;

	/* These are only valid if the payment succeeded.  */
	/* Amount we think reached the payee.  */
	Ln::Amount amount;
	/* Amount we think we released.  */
	Ln::Amount amount_sent;
};

}}

#endif /* !defined(BOSS_MSG_RESPONSELISTPAYS_HPP) */
