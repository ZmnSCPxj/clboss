#ifndef BOSS_MOD_AVAILABLERPCCOMMANDSANNOUNCER_HPP
#define BOSS_MOD_AVAILABLERPCCOMMANDSANNOUNCER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::AvailableRpcCommandsAnnouncer
 *
 * @brief At startup, execute `help` on the node
 * and find out the commands that are available on
 * the C-Lightning node, and announce them via the
 * `Boss::Msg::AvailableRpcCommands` message.
 * This is useful to automatically adapt to changes
 * in version of the C-Lightning node, or the
 * existence/nonexistence of particular plugins.
 */
class AvailableRpcCommandsAnnouncer {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	AvailableRpcCommandsAnnouncer() =delete;

	AvailableRpcCommandsAnnouncer(AvailableRpcCommandsAnnouncer&&);
	~AvailableRpcCommandsAnnouncer();

	explicit
	AvailableRpcCommandsAnnouncer(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_AVAILABLERPCCOMMANDSANNOUNCER_HPP) */
