#ifndef BOSS_MSG_NOTIFICATION_HPP
#define BOSS_MSG_NOTIFICATION_HPP

#include"Jsmn/Object.hpp"
#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::Notification
 *
 * @brief emitted whenever a notification is
 * received on stdin.
 */
struct Notification {
	std::string notification;
	Jsmn::Object params;
};

}}

#endif /* BOSS_MSG_NOTIFICATION_HPP */
