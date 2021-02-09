#ifndef BOSS_MSG_RESPONSESELFUPTIME_HPP
#define BOSS_MSG_RESPONSESELFUPTIME_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponseSelfUptime
 *
 * @brief Broadcast in response to `Boss::Msg::RequestSelfUptime` to
 * provide uptime information.
 */
struct ResponseSelfUptime {
	void* requester;

	/* Uptime in the last 3 days.  */
	double day3;
	/* Uptime in the last 14 days.  */
	double weeks2;
	/* Uptime in the last 30 days.  */
	double month1;
};

}}

#endif /* !defined(BOSS_MSG_RESPONSESELFUPTIME_HPP) */
