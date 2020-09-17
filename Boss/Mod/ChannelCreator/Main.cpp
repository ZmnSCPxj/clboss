#include"Boss/Mod/ChannelCreator/Carpenter.hpp"
#include"Boss/Mod/ChannelCreator/Main.hpp"
#include"Boss/Mod/ChannelCreator/Manager.hpp"
#include"Util/make_unique.hpp"

namespace Boss { namespace Mod { namespace ChannelCreator {

Main::Main( S::Bus& bus
	  , Boss::Mod::Waiter& waiter
	  , Boss::Mod::ChannelCandidateInvestigator::Main& investigator
	  ) : carpenter(Util::make_unique<Carpenter>(bus, waiter))
	    , manager(Util::make_unique<Manager>( bus
						, investigator
						, *carpenter
						))
	    { }
Main::~Main() { }

}}}
