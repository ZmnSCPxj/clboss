#ifndef BOSS_MSG_RESPONSEMOVEFUNDS_HPP
#define BOSS_MSG_RESPONSEMOVEFUNDS_HPP

#include"Ln/Amount.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponseMoveFunds
 *
 * @brief Response from the corresponding
 * `Boss::Msg::RequestMoveFunds` message.
 */
struct ResponseMoveFunds {
	void* requester;

	/* Actual amount we were able to move before giving up.  */
	Ln::Amount amount_moved;
	/* Actual amount spent on fees.  */
	Ln::Amount fee_spent;
};

}}

#endif /* !defined(BOSS_MSG_RESPONSEMOVEFUNDS_HPP) */
