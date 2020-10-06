#ifndef BOSS_MSG_REQUESTPEERSTATISTICS_HPP
#define BOSS_MSG_REQUESTPEERSTATISTICS_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestPeerStatistics
 *
 * @brief emit to request the `Boss::Mod::PeerStatistician`
 * for data from all peers in the given time frame.
 *
 * @desc The given `requester` pointer will be copied in
 * the corresponding `Boss::Msg::ResponsePeerStatistics`.
 * The peer statistician will only keep data for the last
 * three months.
 */
struct RequestPeerStatistics {
	void* requester;
	/* Time frame, in seconds from the epoch.
	 * start_time < end_time.
	 */
	double start_time;
	double end_time;
};

}}

#endif /* !defined(BOSS_MSG_REQUESTPEERSTATISTICS_HPP) */
