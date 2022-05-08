#ifndef BOSS_MSG_RESPONSEREBALANCEUNMANAGED_HPP
#define BOSS_MSG_RESPONSEREBALANCEUNMANAGED_HPP

#include<set>

namespace Ln { class NodeId; }

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponseRebalanceUnmanaged
 *
 * @brief Broadcasted by `Boss::Mod::RebalanceUnmanager` in response
 * to `Boss::Msg::RequestRebalanceUnmanaged`.
 */
struct ResponseRebalanceUnmanaged {
	void* requester;

	std::set<Ln::NodeId> const& unmanaged;
};

}}

#endif /* !defined(BOSS_MSG_RESPONSEREBALANCEUNMANAGED_HPP) */
