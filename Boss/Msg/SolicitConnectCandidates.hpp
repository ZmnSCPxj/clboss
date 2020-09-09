#ifndef BOSS_MSG_SOLICITCONNECTCANDIDATES_HPP
#define BOSS_MSG_SOLICITCONNECTCANDIDATES_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::SolicitConnectCandidates
 *
 * @brief emitted when we want to look for new
 * candidates to connect to.
 *
 * @desc Modules waiting on this message should
 * emit a ProposeConnectCandidates message in
 * response to this message.
 */
struct SolicitConnectCandidates {};

}}

#endif /* BOSS_MSG_SOLICITCONNECTCANDIDATES_HPP */
