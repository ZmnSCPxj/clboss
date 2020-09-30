#ifndef BOSS_MSG_PEERMEDIANCHANNELFEE_HPP
#define BOSS_MSG_PEERMEDIANCHANNELFEE_HPP

#include"Ln/NodeId.hpp"
#include<cstdint>

namespace Boss { namespace Msg {

/** struct Boss::Msg::PeerMedianChannelFee
 *
 * @brief broadcast at `init`, and at the random
 * hourly timer, informing all modules of the
 * median channel fees going *into* our peers.
 */
struct PeerMedianChannelFee {
	Ln::NodeId node;
	std::uint32_t base;
	std::uint32_t proportional;
};

}}

#endif /* !defined(BOSS_MSG_PEERMEDIANCHANNELFEE_HPP) */
