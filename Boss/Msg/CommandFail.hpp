#ifndef BOSS_MSG_COMMANDFAIL_HPP
#define BOSS_MSG_COMMANDFAIL_HPP

#include"Json/Out.hpp"
#include"Ln/CommandId.hpp"
#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::CommandFail
 *
 * @brief Emit in response to a `Boss::Msg::CommandRequest`,
 * to indicate that the command failed.
 * If multiple responses for the same ID are emitted, or
 * for a nonexistent ID, the extras/nonexistent are
 * silently ignored.
 */
struct CommandFail {
	Ln::CommandId id;
	int code;
	std::string message;
	Json::Out data;
};

}}

#endif /* !defined(BOSS_MSG_COMMANDFAIL_HPP) */
