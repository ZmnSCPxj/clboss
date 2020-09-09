#ifndef BOSS_MSG_RESPONSECONNECT_HPP
#define BOSS_MSG_RESPONSECONNECT_HPP

#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponseConnect
 *
 * @brief sent in response to a RequestConnect.
 */
struct ResponseConnect {
	std::string node;
	bool success;
};

}}

#endif /* !defined(BOSS_MSG_RESPONSECONNECT_HPP) */
