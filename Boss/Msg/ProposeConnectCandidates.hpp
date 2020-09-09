#ifndef BOSS_MSG_PROPOSECONNECTCANDIDATES_HPP
#define BOSS_MSG_PROPOSECONNECTCANDIDATES_HPP

#include<string>
#include<vector>

namespace Boss { namespace Msg {

/** struct Boss::Msg::ProposeConnectCandidates
 *
 * @brief propose some connects in response to
 * SolicitConnectCandidates.
 */
struct ProposeConnectCandidates {
	/* Each string should be of the form id@host:port.  */
	std::vector<std::string> candidates;
};

}}

#endif /* !defined(BOSS_MSG_PROPOSECONNECTCANDIDATES_HPP) */
