#ifndef BOSS_MSG_REQUESTSELFUPTIME_HPP
#define BOSS_MSG_REQUESTSELFUPTIME_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestSelfUptime
 *
 * @brief queries our own uptime.
 * The `Boss::Mod::SelfUptimeMonitor` will respond with a
 * `Boss::Msg::ResponseSelfUptime` message.
 */
struct RequestSelfUptime {
	void* requester;
};

}}

#endif /* !defined(BOSS_MSG_REQUESTSELFUPTIME_HPP) */
