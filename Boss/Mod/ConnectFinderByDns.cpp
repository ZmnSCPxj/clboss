#include"Boss/Mod/ConnectFinderByDns.hpp"
#include"Boss/Msg/Begin.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ProposeConnectCandidates.hpp"
#include"Boss/Msg/Network.hpp"
#include"Boss/Msg/SolicitConnectCandidates.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"DnsSeed/get.hpp"
#include"Ev/Io.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<map>
#include<string>
#include<vector>

namespace {

auto const all_dnsseeds =
	std::map<Boss::Msg::Network, std::vector<std::string>>
{ {Boss::Msg::Network_Bitcoin, { "lseed.bitcoinstats.com"
			       /*, "lseed.darosior.ninja" */
			       }}
};

}

namespace Boss { namespace Mod {

class ConnectFinderByDns::Impl {
private:
	S::Bus& bus;

	std::vector<std::string> const* dnsseeds;

	void start() {
		bus.subscribe<Msg::Begin>([this](Msg::Begin const& _) {
			return begin();
		});
	}

	Ev::Io<void> begin() {
		return DnsSeed::can_get().then([this](std::string why) {
			if (why == "")
				return enable();
			return Boss::log( bus, Warn
					, "DnsSeed: Cannot seed by DNS: %s"
					, why.c_str()
					);
		});
	}

	Ev::Io<void> enable() {
		bus.subscribe<Msg::Init>([this](Msg::Init const& init) {
			auto it = all_dnsseeds.find(init.network);
			if ( it != all_dnsseeds.end()
			  && it->second.size() != 0
			   ) {
				dnsseeds = &it->second;
				return Ev::lift();
			}
			return Boss::log( bus, Warn
					, "DnsSeed: Cannot seed by DNS: %s"
					, "No known seeds for this network."
					);
		});
		bus.subscribe<Msg::SolicitConnectCandidates>([this](Msg::SolicitConnectCandidates const& _) {
			return solicit();
		});
		return Ev::lift();
	}

	Ev::Io<void> solicit() {
		if (!dnsseeds || dnsseeds->size() == 0)
			return Ev::lift();
		/* Pick a DNS seed, any DNS seed.  */
		auto& deck = *dnsseeds;
		auto dist = std::uniform_int_distribution<size_t>
				(0, deck.size() - 1);
		auto i = dist(random_engine);
		auto& seed = deck[i];

		return DnsSeed::get(
			seed
		).then([this](std::vector<std::string> ns) {
			return bus.raise(Msg::ProposeConnectCandidates{
				std::move(ns)
			});
		});
	}

public:
	explicit
	Impl( S::Bus& bus_
	    ) : bus(bus_)
	      , dnsseeds(nullptr)
	      {
		start();
	}
};

ConnectFinderByDns::ConnectFinderByDns
		( S::Bus& bus
		) : pimpl(Util::make_unique<Impl>(bus))
		  { }
ConnectFinderByDns::ConnectFinderByDns
		( ConnectFinderByDns&& o
		) : pimpl(std::move(o.pimpl))
		  { }
ConnectFinderByDns::~ConnectFinderByDns() { }

}}
