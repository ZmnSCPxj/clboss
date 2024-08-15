#ifndef BOSS_MSG_RESPONSEEARNINGSINFO_HPP
#define BOSS_MSG_RESPONSEEARNINGSINFO_HPP

#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponseEarningsInfo
 *
 * @brief Emitted in response to a `Boss::Msg::RequestEarningsInfo`,
 * providing information about the queried node.
 */
struct ResponseEarningsInfo {
	void* requester;
	Ln::NodeId node;

	/* How much this has earned in the node->us direction.
	 * i.e. sum of all forwarding fees where this node was
	 * the incoming node.
	 */
	Ln::Amount in_earnings;
	/* How much we have spent on rebalancing *from* this node, which
	 * frees up incoming funds and thus should allow for further
	 * in-earnings.
	 */
	Ln::Amount in_expenditures;

	/* How much this has earned in the us->node direction.  */
	Ln::Amount out_earnings;
	/* How much we have spent on rebalancing *to* this node.  */
	Ln::Amount out_expenditures;

	Ln::Amount in_forwarded;	// Amount forwarded inward to earn fees
	Ln::Amount in_rebalanced; 	// Amount inward rebalanced for expenditure
	Ln::Amount out_forwarded;	// Amount forwarded outward to earn fees
	Ln::Amount out_rebalanced;	// Amount outward rebalanced for expenditure
};

}}

#endif /* !defined(BOSS_MSG_RESPONSEEARNINGSINFO_HPP) */
