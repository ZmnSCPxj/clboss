#ifndef BOSS_MSG_REQUESTREBALANCEBUDGET_HPP
#define BOSS_MSG_REQUESTREBALANCEBUDGET_HPP

#include"Ln/NodeId.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestRebalanceBudget
 *
 * @brief Requests for a fee budget for a
 * rebalance from `source` to `destination`.
 */
struct RequestRebalanceBudget {
	void* requester;
	Ln::NodeId source;
	Ln::NodeId destination;
};

}}

#endif /* !defined(BOSS_MSG_REQUESTREBALANCEBUDGET_HPP) */
