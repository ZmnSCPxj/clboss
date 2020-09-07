#ifndef BOSS_MSG_MANIFESTCOMMAND_HPP
#define BOSS_MSG_MANIFESTCOMMAND_HPP

#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::ManifestCommand
 *
 * @brief emitted while handling a Boss::Msg::Manifestation
 * in order to register a command.
 */
struct ManifestCommand {
	std::string name;
	std::string usage;
	std::string description;
	bool deprecated;
};

}}

#endif /* !defined(BOSS_MSG_MANIFESTCOMMAND_HPP) */
