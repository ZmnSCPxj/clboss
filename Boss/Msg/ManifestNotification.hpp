#ifndef BOSS_MSG_MANIFESTNOTIFICATION_HPP
#define BOSS_MSG_MANIFESTNOTIFICATION_HPP

#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::ManifestNotification
 *
 * @brief emitted in respone to Boss::Msg::Manifestation
 * to register a notification.
 * Multiple modules can register the same notification
 * and listen for Boss::Msg::Notification.
 */
struct ManifestNotification {
	std::string name;
};

}}

#endif /* !defined(BOSS_MSG_MANIFESTNOTIFICATION_HPP) */
