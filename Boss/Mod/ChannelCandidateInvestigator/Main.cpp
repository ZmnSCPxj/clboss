#include"Boss/Mod/ChannelCandidateInvestigator/Gumshoe.hpp"
#include"Boss/Mod/ChannelCandidateInvestigator/Main.hpp"
#include"Boss/Mod/ChannelCandidateInvestigator/Manager.hpp"
#include"Boss/Mod/ChannelCandidateInvestigator/Secretary.hpp"
#include"Util/make_unique.hpp"

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator {

Main::Main(Main&& o) : secretary(std::move(o.secretary))
		     , gumshoe(std::move(o.gumshoe))
		     , manager(std::move(o.manager))
		     { }
Main::~Main() {}

Main::Main(S::Bus& bus
	  ) : secretary(Util::make_unique<Secretary>())
	    , gumshoe(Util::make_unique<Gumshoe>(bus))
	    {
	manager = Util::make_unique<Manager>( bus
					    , *secretary
					    , *gumshoe
					    );
}

}}}
