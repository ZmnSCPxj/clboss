#ifndef BOSS_MSG_REQUESTREBALANCEUNMANAGED_HPP
#define BOSS_MSG_REQUESTREBALANCEUNMANAGED_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestRebalanceUnmanaged
 *
 * @brief Requests the `Boss::Mod::RebalanceUnmanager` to provide
 * the list of unmanaged-for-rebalance nodes.
 */
struct RequestRebalanceUnmanaged {
	void* requester;
};

}}

#endif /* !defined(BOSS_MSG_REQUESTREBALANCEUNMANAGED_HPP) */
