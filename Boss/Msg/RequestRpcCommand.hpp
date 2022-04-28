#ifndef BOSS_MSG_REQUESTRPCCOMMAND_HPP
#define BOSS_MSG_REQUESTRPCCOMMAND_HPP

#include"Json/Out.hpp"
#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestRpcCommand
 *
 * @brief Requests an RPC command to be sent to the
 * `lightningd`.
 * This is handled by the `Boss::Mod::RpcWrapper`.
 * The result of the command will then be broadcast
 * via `Boss::Msg::ResponseRpcCommand`.
 */
struct RequestRpcCommand {
	/*~ The object that requested the RPC command.  */
	void* requester;
	/*~ The command to execute.  */
	std::string command;
	/*~ The parameters to the command.  */
	Json::Out params;
};

}}

#endif /* BOSS_MSG_REQUESTRPCCOMMAND_HPP */
