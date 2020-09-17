#ifndef BOSS_MSG_CHANNELCREATERESULT_HPP
#define BOSS_MSG_CHANNELCREATERESULT_HPP

#include"Ln/NodeId.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ChannelCreateResult
 *
 * @brief used to report that we attempted a
 * channel open with the node, and whether
 * it succeeded or failed.
 */
struct ChannelCreateResult {
	Ln::NodeId node;
	bool success;
};

}}

#endif /* !defined(BOSS_MSG_CHANNELCREATERESULT_HPP) */

