#include"Boss/Mod/RegularActiveProbe.hpp"
#include"Boss/Msg/ListpeersAnalyzedResult.hpp"
#include"Boss/Msg/ProbeActively.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
#include"Ev/foreach.hpp"
#include"S/Bus.hpp"
#include<algorithm>
#include<iterator>
#include<random>

namespace {

/* If we call dist on the random engine, and it comes up
 * 1, we trigger for a particular peer.
 * This comes up every 10 minutes.
 */
auto dist = std::uniform_int_distribution<std::size_t>(
	1, 144
);

}

namespace Boss { namespace Mod {

void RegularActiveProbe::start() {
	bus.subscribe<Msg::ListpeersAnalyzedResult
		     >([this](Msg::ListpeersAnalyzedResult const& m) {
		auto peers = std::vector<Ln::NodeId>();
		std::copy( m.connected_channeled.begin()
			 , m.connected_channeled.end()
			 , std::back_inserter(peers)
			 );
		std::copy( m.disconnected_channeled.begin()
			 , m.disconnected_channeled.end()
			 , std::back_inserter(peers)
			 );
		auto f = [this](Ln::NodeId peer) {
			if (dist(Boss::random_engine) != 1)
				return Ev::lift();
			auto peer_s = std::string(peer);
			return Boss::log( bus, Debug
					, "RegularActiveProbe: Won for peer "
					  "%s."
					, peer_s.c_str()
					)
			     + bus.raise(Msg::ProbeActively{std::move(peer)})
			     ;
		};
		return Boss::log( bus, Debug
				, "RegularActiveProbe: Rolling dice for "
				  "all peers."
				)
		     + Ev::foreach(std::move(f), std::move(peers))
		     ;
	});
}

}}
