#include"Boss/Mod/ChannelCreator/Main.hpp"
#include"Boss/Mod/ChannelCreator/Manager.hpp"
#include"Util/make_unique.hpp"

namespace Boss { namespace Mod { namespace ChannelCreator {

/* TODO Carpenter.  */
struct Carpenter {};

Main::Main( S::Bus& bus
	  , Boss::Mod::ChannelCandidateInvestigator::Main& investigator
	  ) : manager(Util::make_unique<Manager>(bus, investigator))
	    { }
Main::~Main() { }

}}}
