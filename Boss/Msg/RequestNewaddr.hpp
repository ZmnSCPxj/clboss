#ifndef BOSS_MSG_REQUESTNEWADDR_HPP
#define BOSS_MSG_REQUESTNEWADDR_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestNewaddr
 *
 * @brief emitted to request for a new
 * address.
 */
struct RequestNewaddr {
	void* requester;
};

}}

#endif /* !defined(BOSS_MSG_REQUESTNEWADDR_HPP) */
