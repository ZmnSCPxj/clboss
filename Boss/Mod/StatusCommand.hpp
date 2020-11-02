#ifndef BOSS_MOD_STATUSCOMMAND_HPP
#define BOSS_MOD_STATUSCOMMAND_HPP

#include"Json/Out.hpp"
#include<cstddef>
#include<map>

namespace Boss { namespace Msg { struct CommandRequest; }}
namespace Boss { namespace Msg { struct Manifestation; }}
namespace Boss { namespace Msg { struct ProvideStatus; }}
namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::StatusCommand
 *
 * @brief implements the `clboss-status` command,
 * soliciting status reports from other modules
 * and then providing them to the user.
 */
class StatusCommand {
private:
	S::Bus& bus;
	std::uint64_t id;
	bool soliciting;
	std::map<std::string, Json::Out> fields;

	void start();
	Ev::Io<void> on_manifest(Boss::Msg::Manifestation const& m);
	Ev::Io<void> on_command(Boss::Msg::CommandRequest const& c);
	Ev::Io<void> on_status(Boss::Msg::ProvideStatus const& s);

public:
	StatusCommand() =delete;
	StatusCommand(StatusCommand const&) =delete;
	StatusCommand(StatusCommand&&) =delete;

	explicit
	StatusCommand( S::Bus& bus_
		     ) : bus(bus_)
		       , id()
		       , soliciting(false)
		       , fields()
		       { start(); }
};

}}

#endif /* !defined(BOSS_MOD_STATUSCOMMAND_HPP) */
