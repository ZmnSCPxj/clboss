#ifndef BOSS_MSG_RESPONSERPCCOMMAND_HPP
#define BOSS_MSG_RESPONSERPCCOMMAND_HPP

#include"Jsmn/Object.hpp"
#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponseRpcCommand
 *
 * @brief Broadcast in response to `Boss::Msg::RequestRpcCommand`,
 * after the corresponding command has succeeded or failed.
 */
struct ResponseRpcCommand {
	/*~ The object that requested the RPC command.  */
	void* requester;
	/*~ True if the command succeded.  */
	bool succeeded;
	/*~ Result if succeeded, or the error if failed.  */
	Jsmn::Object result;
	/*~ The command that failed if it failed, empty if the
	 * command succeeded.
	 */
	std::string command;
};

}}

#endif /* BOSS_MSG_RESPONSERPCCOMMAND_HPP */
