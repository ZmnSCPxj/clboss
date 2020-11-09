#ifndef BOSS_MSG_RPCCOMMANDHOOK_HPP
#define BOSS_MSG_RPCCOMMANDHOOK_HPP

#include"Jsmn/Object.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::RpcCommandHook
 *
 * @brief Emitted in response to the `lightningd` receiving
 * an RPC command.
 *
 * @desc This does not require or wait for a response: the
 * `Boss::Mod::CommandReceiver` specially treats `rpc_command`
 * hooks and responds to those immediately.
 * After responding, the `CommandReceiver` emits this message.
 */
struct RpcCommandHook {
	Jsmn::Object params;
};

}}

#endif /* !defined(BOSS_MSG_RPCCOMMANDHOOK_HPP) */
