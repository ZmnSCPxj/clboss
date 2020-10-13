#ifndef BOSS_MSG_CHANNELCREATION_HPP
#define BOSS_MSG_CHANNELCREATION_HPP

#include"Ln/NodeId.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ChannelCreation
 *
 * @brief Tells everyone that we now have a new channel
 * with a peer.
 * This applies regardless of whether it was us who
 * initiated the channel, or the peer.
 * See also `Boss::Msg::ChannelDestruction`.
 */
struct ChannelCreation {
	Ln::NodeId peer;
};

}}

#endif /* !defined(BOSS_MSG_CHANNELCREATION_HPP) */
