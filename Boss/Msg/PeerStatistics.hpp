#ifndef BOSS_MSG_PEERSTATISTICS_HPP
#define BOSS_MSG_PEERSTATISTICS_HPP

#include"Ln/Amount.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::PeerStatistics
 *
 * @brief this is not a message, but is used in
 * `Boss::Msg::ResponsePeerStatistics`.
 * It contains statistics for a single peer.
 */
struct PeerStatistics {
	/* Actual time frame covered.
	 * If you requested time from today to three days ago,
	 * and the peer only existed with a channel yesterday,
	 * then the below will be different from the data you
	 * specified in `Boss::Msg::RequestPeerStatistics`.
	 *
	 * These are in seconds from the epoch.
	 */
	double start_time;
	double end_time;

	/* Age of our channel(s) with the peer.
	 * This is independent of the requested time frame.
	 * Unit is seconds.
	 */
	double age;

	/* Total amount of time that our outgoing payments via
	 * this peer have been locked.
	 * Unit is seconds.
	 */
	double lockrealtime;
	/* Total number of outgoing payments we made via this
	 * peer.  */
	std::size_t attempts;
	/* Total number of times outgoing payments we made reached
	 * the destination, when made via this peer.
	 */
	std::size_t successes;

	/* Total number of times we saw the peer as connected.  */
	std::size_t connects;
	/* Total number of times we checked if the peer was connected.  */
	std::size_t connect_checks;

	/* Total fees we definitely earned from incoming forwards from
	 * this peer.  */
	Ln::Amount in_fee;
	/* Total fees we definitely earned from outgoing forwards to
	 * this peer.  */
	Ln::Amount out_fee;
};

}}

#endif /* !defined(BOSS_MSG_PEERSTATISTICS_HPP) */
