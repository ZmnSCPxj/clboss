#ifndef BOSS_MSG_RESPONSEPEERSTATISTICS_HPP
#define BOSS_MSG_RESPONSEPEERSTATISTICS_HPP

#include"Boss/Msg/PeerStatistics.hpp"
#include"Ln/NodeId.hpp"
#include<map>

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponsePeerStatistics
 *
 * @brief Raised in response to `Boss::Msg::RequestPeerStatistics`.
 * Contains all data about all peers that were present in the
 * requested time frame.
 */
struct ResponsePeerStatistics {
	void* requester;

	std::map<Ln::NodeId, PeerStatistics> statistics;
};

}}

#endif /* !defined(BOSS_MSG_RESPONSEPEERSTATISTICS_HPP) */
