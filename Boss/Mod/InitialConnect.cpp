#include"Boss/Mod/InitialConnect.hpp"
#include"Boss/Msg/ListpeersAnalyzedResult.hpp"
#include"Boss/Msg/NeedsConnect.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include<assert.h>

namespace Boss { namespace Mod {

void InitialConnect::start() {
	bus.subscribe<Boss::Msg::ListpeersAnalyzedResult
		     >([this](Boss::Msg::ListpeersAnalyzedResult const& r) {
		if (r.initial)
			return init_check(r);
		else
			return periodic_check(r);
	});
}

Ev::Io<void>
InitialConnect::init_check(Msg::ListpeersAnalyzedResult const& r) {
	/* If there are any channeled peers, it is likely that
	 * `lightningd` will naturally connect to them.  */
	if ( !r.connected_channeled.empty()
	  || !r.disconnected_channeled.empty()
	   )
		return Ev::lift();

	/* Otherwise there are no peers with live channels, so
	 * we shold try to live them.  */
	return needs_connect("No peers with live channels.");
}

Ev::Io<void>
InitialConnect::periodic_check(Msg::ListpeersAnalyzedResult const& r) {
	/* If there are any connected peers, no need to get more.  */
	if ( !r.connected_channeled.empty()
	  || !r.connected_unchanneled.empty()
	   )
		return Ev::lift();

	return needs_connect("No connected peers.");
}

Ev::Io<void> InitialConnect::needs_connect(std::string reason) {
	return Boss::log( bus, Debug
			, "InitialConnect: Needs connect: %s"
			, reason.c_str()
			).then([this]() {
		return Boss::concurrent(bus.raise(Boss::Msg::NeedsConnect()));
	});
}

}}
