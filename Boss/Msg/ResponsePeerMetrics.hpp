#ifndef BOSS_MSG_RESPONSEPEERMETRICS_HPP
#define BOSS_MSG_RESPONSEPEERMETRICS_HPP

#include"Boss/Msg/PeerMetrics.hpp"
#include"Ln/NodeId.hpp"
#include<map>

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponsePeerMetrics
 *
 * @brief eventually emitted in response to a
 * `Boss::Msg::RequestPeerMetrics` with the
 * `requester` field copied.
 *
 * @desc contains metrics from recorded statistical
 * data for the last 3 days, 2 weeks, and month.
 */
struct ResponsePeerMetrics {
	void* requester;

	std::map<Ln::NodeId, PeerMetrics> day3;
	std::map<Ln::NodeId, PeerMetrics> weeks2;
	std::map<Ln::NodeId, PeerMetrics> month1;
};

}}

#endif /* !defined(BOSS_MSG_RESPONSEPEERMETRICS_HPP) */
