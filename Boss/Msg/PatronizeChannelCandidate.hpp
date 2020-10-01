#ifndef BOSS_MSG_PATRONIZECHANNELCANDIDATE_HPP
#define BOSS_MSG_PATRONIZECHANNELCANDIDATE_HPP

#include"Ln/NodeId.hpp"
#include<vector>

namespace Boss { namespace Msg {

/** struct Boss::Msg::PatronizeChannelCandidate
 *
 * @brief emitted asynchronously in response to
 * `Boss::Msg::ProposePatronlessChannelCandidate`.
 *
 * @desc The channel investigator emits this
 * message, which triggers the matchmaker to figure
 * out a patron using the given nodes as guides.
 */
struct PatronizeChannelCandidate {
	Ln::NodeId proposal;
	/* A set of nodes which might be a good idea to
	 * try to target.
	 * These will not necessarily become the patron,
	 * but are used as guides to derive *some* patron
	 * for the proposal.
	 */
	std::vector<Ln::NodeId> guide;
};

}}

#endif /* !defined(BOSS_MSG_PATRONIZECHANNELCANDIDATE_HPP) */
