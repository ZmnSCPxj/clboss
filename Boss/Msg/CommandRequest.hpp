#ifndef BOSS_MSG_COMMANDREQUEST_HPP
#define BOSS_MSG_COMMANDREQUEST_HPP

#include"Jsmn/Object.hpp"
#include"Ln/CommandId.hpp"
#include<cstdint>
#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::CommandRequest
 *
 * @brief emitted whenever a command or hook is
 * received on stdin.
 * Respond by Boss::Msg::CommandResponse.
 */
struct CommandRequest {
	std::string command;
	Jsmn::Object params;
	Ln::CommandId id;
};

}}

#endif /* !defined(BOSS_MSG_COMMANDREQUEST_HPP) */
