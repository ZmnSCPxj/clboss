#ifndef BOSS_MSG_PROPOSECHANNELCANDIDATES_HPP
#define BOSS_MSG_PROPOSECHANNELCANDIDATES_HPP

#include"Ln/NodeId.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ProposeChannelCandidates
 *
 * @brief emit in order to propose the given
 * node as one we should make channels with.
 *
 * @desc Two nodes must be put in the message.
 * One is the actual node to propose connecting
 * to.
 * The other one, the patron, is a "goal" node
 * that is the reason we are proposing the
 * proposal node in the first place.
 *
 * When we later actually make a channel with
 * the proposal node, we estimate how much
 * capacity is available towards the patron
 * in order to provide a cap on the capacity
 * of the proposal node.
 *
 * See also `Boss::Msg::PreinvestigateChannelCandidates`.
 */
struct ProposeChannelCandidates {
	Ln::NodeId proposal;
	Ln::NodeId patron;
};

}}

#endif /* !defined(BOSS_MSG_PROPOSECHANNELCANDIDATES_HPP) */

