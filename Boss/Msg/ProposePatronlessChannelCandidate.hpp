#ifndef BOSS_MSG_PROPOSEPATRONLESSCHANNELCANDIDATE_HPP
#define BOSS_MSG_PROPOSEPATRONLESSCHANNELCANDIDATE_HPP

#include"Ln/NodeId.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ProposePatronlessChannelCandidate
 *
 * @brief emit this to propose a channel candidate
 * and delegate figuring out a patron for it to
 * some other module.
 *
 * @desc this is not the preferred way of proposing
 * channel candidates, as we need some way of guessing
 * how much would be a reasonable amount to put into a
 * channel with a candidate.
 *
 * Patrons are how we guess that --- if the candidate
 * has good capacity, direct or otherwise, going to
 * the patron, then we have sense it is good to open
 * larger channels to it.
 */
struct ProposePatronlessChannelCandidate {
	Ln::NodeId proposal;
};

}}

#endif /* !defined(BOSS_MSG_PROPOSEPATRONLESSCHANNELCANDIDATE_HPP) */
