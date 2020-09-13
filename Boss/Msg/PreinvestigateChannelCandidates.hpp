#ifndef BOSS_MSG_PREINVESTIGATECHANNELCANDIDATES_HPP
#define BOSS_MSG_PREINVESTIGATECHANNELCANDIDATES_HPP

#include"Boss/Msg/ProposeChannelCandidates.hpp"
#include<cstddef>
#include<vector>

namespace Boss { namespace Msg {

/** struct Boss::Msg::PreinvestigateChannelCandidates
 *
 * @brief emit in order to preinvestigate a set of channel
 * candidates.
 *
 * @desc The preinvestigator will accept this message, then
 * connect to each listed `proposal` node, one at a time.
 * If it successfullly `connect`s, the preinvestigator will
 * emit a `ProposeChannelCandidates` for the main
 * investigator.
 *
 * The preinvestigator will emit only up to `max_candidates`
 * successfully-connected candidates.
 */
struct PreinvestigateChannelCandidates {
	std::vector<ProposeChannelCandidates> candidates;
	std::size_t max_candidates;
};

}}

#endif /* !defined(BOSS_MSG_PREINVESTIGATECHANNELCANDIDATES_HPP) */

