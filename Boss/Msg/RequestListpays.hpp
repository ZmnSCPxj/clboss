#ifndef BOSS_MSG_REQUESTLISTPAYS_HPP
#define BOSS_MSG_REQUESTLISTPAYS_HPP

#include"Sha256/Hash.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestListpays
 *
 * @brief emitted whenever some module wants
 * to know the status of some payment.
 *
 * @desc the listpays handler will emit a
 * `Boss::Msg::ResponseListpays` in response
 * to this.
 */
struct RequestListpays {
	/* What about a future with payment points???  */
	Sha256::Hash payment_hash;
};

}}

#endif /* !defined(BOSS_MSG_REQUESTLISTPAYS_HPP) */
