#ifndef BOSS_MSG_NEEDSCONNECT_HPP
#define BOSS_MSG_NEEDSCONNECT_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::NeedsConnect
 *
 * @brief emitted whenever we think we need to
 * make a few connections.
 *
 * @desc Emitted whenever we need connections.
 * Note that the purpose of the connection is
 * not to make channels, but to get gossip in
 * order to learn more about the network.
 */
struct NeedsConnect { };

}}

#endif /* !defined(BOSS_MSG_NEEDSCONNECT_HPP) */
