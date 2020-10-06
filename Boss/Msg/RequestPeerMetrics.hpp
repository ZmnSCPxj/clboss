#ifndef BOSS_MSG_REQUESTPEERMETRICS_HPP
#define BOSS_MSG_REQUESTPEERMETRICS_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestPeerMetrics
 *
 * @brief emit this in order to request peer metrics.
 */
struct RequestPeerMetrics {
	/* Used to identify the object requesting
	 * for peer metrics.
	 * Will be copied in the corresponding
	 * `Boss::Msg::ResponsePeerMetrics`.
	 */
	void* requester;
};

}}

#endif /* !defined(BOSS_MSG_REQUESTPEERMETRICS_HPP) */
