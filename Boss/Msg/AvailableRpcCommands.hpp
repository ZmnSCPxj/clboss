#ifndef BOSS_MSG_AVAILABLERPCCOMMANDS_HPP
#define BOSS_MSG_AVAILABLERPCCOMMANDS_HPP

#include<map>
#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::AvailableRpcCommands
 *
 * @brief Broadcast at startup, containing the commands
 * available on the managed C-Lightning node, plus the
 * usage of the commands.
 */
struct AvailableRpcCommands {
	struct Desc {
		std::string category;
		/*~ Does not include command name.  */
		std::string usage;
	};
	/*~ Key is the command name.  */
	std::map<std::string, Desc> commands;
};

}}

#endif /* BOSS_MSG_AVAILABLERPCCOMMANDS_HPP */
