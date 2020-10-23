#ifndef BOSS_MSG_REQUESTEARNINGSINFO_HPP
#define BOSS_MSG_REQUESTEARNINGSINFO_HPP

#include"Ln/NodeId.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestEarningsInfo
 *
 * @brief Requests to get information about fee earnings, as well
 * as debits due to rebalancings, about a node.
 */
struct RequestEarningsInfo {
	void* requester;
	Ln::NodeId node;
};

}}

#endif /* !defined(BOSS_MSG_REQUESTEARNINGSINFO_HPP) */
