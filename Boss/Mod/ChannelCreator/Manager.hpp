#ifndef BOSS_MOD_CHANNELCREATOR_MANAGER_HPP
#define BOSS_MOD_CHANNELCREATOR_MANAGER_HPP

#include"Ln/NodeId.hpp"

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator {
	class Main;
}}}
namespace Boss { namespace Mod { namespace ChannelCreator {
	class Carpenter;
}}}
namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
namespace Ln { class Amount; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace ChannelCreator {

/** class Boss::Mod::ChannelCreator::Manager
 *
 * @brief submodule that handles incoming
 * messages to the channel creator, then
 * drives the rest of the processing.
 */
class Manager {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;
	Boss::Mod::ChannelCandidateInvestigator::Main& investigator;
	Ln::NodeId self;

	void start();
	Ev::Io<void> on_request_channel_creation(Ln::Amount);

public:
	Manager() =delete;
	Manager(Manager const&) =delete;
	Manager(Manager&&) =delete;

	/* TODO: require carpenter.  */
	explicit
	Manager( S::Bus& bus_
	       , Boss::Mod::ChannelCandidateInvestigator::Main& investigator_
	       ) : bus(bus_)
		 , rpc(nullptr)
		 , investigator(investigator_)
		 , self()
		 {
		start();
	}
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCREATOR_MANAGER_HPP) */
