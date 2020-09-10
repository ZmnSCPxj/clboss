#ifndef BOSS_MSG_SOLICITCHANNELCANDIDATES_HPP
#define BOSS_MSG_SOLICITCHANNELCANDIDATES_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::SolicitChannelCandidates
 *
 * @brief emitted whenever we think we need
 * new channel candidates.
 *
 * @desc Unlike SolicitConnectCandidates, modules
 * do not need to immediately respond with
 * ProposeChannelCandidates;
 * they can run in the background and then emit
 * proposals later.
 *
 * Channel candidates need to be evaluated for a
 * while first before we consider them ready for
 * making channels with.
 */
struct SolicitChannelCandidates { };

}}

#endif /* !defined(BOSS_MSG_SOLICITCHANNELCANDIDATES_HPP) */
