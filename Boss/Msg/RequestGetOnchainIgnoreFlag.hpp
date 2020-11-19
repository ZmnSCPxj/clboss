#ifndef BOSS_MSG_REQUESTGETONCHAINIGNOREFLAG_HPP
#define BOSS_MSG_REQUESTGETONCHAINIGNOREFLAG_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestGetOnchainIgnoreFlag
 *
 * @brief emit in order to query if we should be ignoring onchain
 * funds now or not.
 * This will be replied to with `ResponseGetOnchainIgnoreFlag`.
 */
struct RequestGetOnchainIgnoreFlag {
	void* requester;
};

}}

#endif /* BOSS_MSG_REQUESTGETONCHAINIGNOREFLAG_HPP */
