#ifndef BOSS_MOD_CHANNELCREATOR_MANAGER_HPP
#define BOSS_MOD_CHANNELCREATOR_MANAGER_HPP

#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/RequestDowser.hpp"
#include"Boss/Msg/ResponseDowser.hpp"
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
	Boss::Mod::ChannelCreator::Carpenter& carpenter;
	Ln::NodeId self;

	ModG::ReqResp<Msg::RequestDowser, Msg::ResponseDowser> dowser;

	void start();
	Ev::Io<void> on_request_channel_creation(Ln::Amount);

public:
	Manager() =delete;
	Manager(Manager const&) =delete;
	Manager(Manager&&) =delete;

	explicit
	Manager( S::Bus& bus_
	       , Boss::Mod::ChannelCandidateInvestigator::Main& investigator_
	       , Boss::Mod::ChannelCreator::Carpenter& carpenter_
	       ) : bus(bus_)
		 , rpc(nullptr)
		 , investigator(investigator_)
		 , carpenter(carpenter_)
		 , self()
		 , dowser( bus
			 , [](Msg::RequestDowser& r, void* p) {
				r.requester = p;
			   }
			 , [](Msg::ResponseDowser& r) {
				return r.requester;
			   }
			 )
		 {
		start();
	}
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCREATOR_MANAGER_HPP) */
