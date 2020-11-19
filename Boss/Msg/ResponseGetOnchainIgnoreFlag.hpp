#ifndef BOSS_MSG_RESPONSEGETONCHAINIGNOREFLAG_HPP
#define BOSS_MSG_RESPONSEGETONCHAINIGNOREFLAG_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponseGetOnchainIgnoreFlag
 *
 * @brief Emitted in response to `RequestGetOnchainIgnoreFlag`.
 */
struct ResponseGetOnchainIgnoreFlag {
	void* requester;
	/* Set if we should be ignoring onchain funds.  */
	bool ignore;
	/* If ignore is true, how many more seconds to be waiting.  */
	double seconds;
};

}}

#endif /* BOSS_MSG_RESPONSEGETONCHAINIGNOREFLAG_HPP */
