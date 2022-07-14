#ifndef BOSS_MSG_RESPONSEREBALANCEBUDGET_HPP
#define BOSS_MSG_RESPONSEREBALANCEBUDGET_HPP

#include"Ln/Amount.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponseRebalanceBudget
 *
 * @brief Returns the amount to put as the
 * fee budget to rebalances.
 */
struct ResponseRebalanceBudget {
	void* requester;
	Ln::Amount fee_budget;
};

}}

#endif /* !defined(BOSS_MSG_RESPONSEREBALANCEBUDGET_HPP) */
