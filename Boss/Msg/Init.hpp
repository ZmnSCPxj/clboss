#ifndef BOSS_MSG_INIT_HPP
#define BOSS_MSG_INIT_HPP

#include"Boss/Msg/Network.hpp"
#include"Ln/NodeId.hpp"
#include"Sqlite3/Db.hpp"

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
	Ln::NodeId self_id;
	Sqlite3::Db db;
};

}}

#endif /* !defined(BOSS_MSG_INIT_HPP) */
