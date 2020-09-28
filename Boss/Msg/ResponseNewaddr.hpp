#ifndef BOSS_MSG_RESPONSENEWADDR_HPP
#define BOSS_MSG_RESPONSENEWADDR_HPP

#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponseNewaddr
 *
 * @brief emitted in response to a
 * `Boss::Msg::RequestNewaddr`
 */
struct ResponseNewaddr {
	std::string address;
	void* requester;
};

}}

#endif /* !defined(BOSS_MSG_RESPONSENEWADDR_HPP) */
