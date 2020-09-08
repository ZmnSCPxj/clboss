#ifndef BOSS_MSG_INIT_HPP
#define BOSS_MSG_INIT_HPP

#include"Boss/Msg/Network.hpp"

namespace Boss { namespace Mod { class Rpc; }}

namespace Boss { namespace Msg {

/** struct Boss::Msg::Init
 *
 * @brief emitted when the `init` command is
 * performed.
 */
struct Init {
	Network network;
	Boss::Mod::Rpc& rpc;
};

}}

#endif /* !defined(BOSS_MSG_INIT_HPP) */
