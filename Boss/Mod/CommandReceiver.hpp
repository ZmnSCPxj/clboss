#ifndef BOSS_MOD_COMMANDRECEIVER_HPP
#define BOSS_MOD_COMMANDRECEIVER_HPP

#include"Ln/CommandId.hpp"
#include<cstdint>
#include<set>

namespace S { class Bus; }

namespace Boss { namespace Mod {

class CommandReceiver {
private:
	S::Bus& bus;

	std::set<Ln::CommandId> pendings;

public:
	explicit
	CommandReceiver(S::Bus& bus_);
};

}}

#endif /* !defined(BOSS_MOD_COMMANDRECEIVER_HPP) */
