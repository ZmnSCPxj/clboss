#include"Boss/Mod/ChannelCandidateInvestigator/Gumshoe.hpp"
#include"Boss/Mod/ChannelCandidateInvestigator/Janitor.hpp"
#include"Boss/Mod/ChannelCandidateInvestigator/Main.hpp"
#include"Boss/Mod/ChannelCandidateInvestigator/Manager.hpp"
#include"Boss/Mod/ChannelCandidateInvestigator/Secretary.hpp"
#include"Ev/Io.hpp"
#include"Util/make_unique.hpp"

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator {

Main::Main(Main&& o) : secretary(std::move(o.secretary))
		     , janitor(std::move(o.janitor))
		     , gumshoe(std::move(o.gumshoe))
		     , manager(std::move(o.manager))
		     { }
Main::~Main() {}

Main::Main( S::Bus& bus
	  , InternetConnectionMonitor& imon
	  ) : secretary(Util::make_unique<Secretary>())
	    , janitor(Util::make_unique<Janitor>(bus))
	    , gumshoe(Util::make_unique<Gumshoe>(bus))
	    {
	manager = Util::make_unique<Manager>( bus
					    , *secretary
					    , *janitor
					    , *gumshoe
					    , imon
					    );
}
Ev::Io<std::vector<std::pair<Ln::NodeId, Ln::NodeId>>>
Main::get_channel_candidates() {
	return manager->get_channel_candidates();
}

}}}
