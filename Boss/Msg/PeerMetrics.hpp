#ifndef BOSS_MSG_PEERMETRICS_HPP
#define BOSS_MSG_PEERMETRICS_HPP

#include<memory>

namespace Boss { namespace Msg {

/** struct Boss::Msg::PeerMetrics
 *
 * @brief this is not an emitted message, but is instead used
 * in `Boss::Msg::ResponsePeerMetrics` to store the metrics
 * of each peer.
 */
struct PeerMetrics {
	/* Age of our channel with this peer, in seconds.  */
	double age;
	/* Average amount of time our payment gets locked by this peer
	 * before it gets either failed or succeeded.
	 * 0 if we never attempted with this peer.
	 */
	double seconds_per_attempt;
	/* Rate at which an outgoing payment via this peer reaches the
	 * destination.
	 * 1.0 = 100%.
	 * nullptr if we have never attempted this peer within the
	 * time frame of this peer metric.
	 */
	std::shared_ptr<double> success_per_attempt;
	/* Average number of times we reached the destination when
	 * sending out via this peer, per day.
	 * 0 if no successes with this peer.
	 */
	double success_per_day;
	/* Rate at which we saw the peer connected in `listpeers`.
	 * 1.0 = 100%.
	 * nullptr if we have not made any connectivity checks in
	 * the time frame of this peer metric.
	 */
	std::shared_ptr<double> connect_rate;
	/* Fees we earned from incoming forwards from this peer,
	 * divided by time frame.
	 * Unit is millisatoshi per day.
	 * 0 if we never had incoming forwards from this peer in
	 * this time frame.
	 */
	double in_fee_msat_per_day;
	/* Fees we earned from outgoing forwards to this peer,
	 * divided by time frame.
	 */
	double out_fee_msat_per_day;

};

}}

#endif /* !defined(BOSS_MSG_PEERMETRICS_HPP) */
