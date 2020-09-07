#ifndef BOSS_MSG_COMMANDRESPONSE_HPP
#define BOSS_MSG_COMMANDRESPONSE_HPP

#include"Json/Out.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::CommandResponse
 *
 * @brief Emit in response to a Boss::Msg::CommandRequest.
 * If multiple responses for the same ID are emitted, or
 * for a nonexistent ID, the extras/nonexistent are
 * silently ignored.
 */
struct CommandResponse {
	std::uint64_t id;
	Json::Out response;
};

}}

#endif /* !defined(BOSS_MSG_COMMANDRESPONSE_HPP) */
