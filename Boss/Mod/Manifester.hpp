#ifndef BOSS_MOD_MANIFESTER_HPP
#define BOSS_MOD_MANIFESTER_HPP

#include"Boss/Msg/ManifestCommand.hpp"
#include<map>
#include<set>
#include<string>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::Manifester
 *
 * @brief Makes the plugin manifest itself.
 * Responds to `getmanifest` commands.
 */
class Manifester {
private:
	S::Bus& bus;
	std::map<std::string, Boss::Msg::ManifestCommand> commands;
	std::set<std::string> hooks;
	std::set<std::string> notifications;

	void start();

public:
	explicit
	Manifester(S::Bus& bus_) : bus(bus_) { start(); }
};

}}

#endif /* !defined(BOSS_MOD_MANIFESTER_HPP) */
