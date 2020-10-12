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
#include<utility>
#include<vector>

namespace {

auto const all_dnsseeds =
	std::map<Boss::Msg::Network, std::vector<std::pair< std::string
							  , std::string
							  >>>
{ {Boss::Msg::Network_Bitcoin, { {"1.0.0.1", "lseed.bitcoinstats.com"}
			       , {"8.8.8.8", "lseed.darosior.ninja"}
			       }}
};
/*
<zmnscpxj> What other Lightning DNS seeds are there? lseed.darosior.ninja is not reliable from my connection, even with @1.1.1.1 or @8.8.8.8
<zmnscpxj> lseed.bitcoinstats.com works with @1.1.1.1
<zmnscpxj> I mean for SRV requests
* jonatack (~jon@37.167.109.160) has joined
* yzernik (~yzernik@12.203.56.101) has joined
<cdecker> zmnscpxj: that's usually an issue with the size of the response, since recursive resolvers cache the result they'll refuse to relay large results.
<cdecker> In my experience around 5 SRV results still work with most recursive resolvers, any more than that and they start simply timing out the requests
<cdecker> Not sure how many darosior has configured
<zmnscpxj> dig @1.1.1.1 n5.lseed.darosior.ninja SRV still gives me no answers
<zmnscpxj> I used to be able to get 25 when using @8.8.8.8 but no longer recently, hmmm
<zmnscpxj> using my ISP resolver gives me 0 results on bitcoinstats.com, I have to use @1.1.1.1
<cdecker> Yes, ISP resolvers are notoriously flaky
<zmnscpxj> dig @1.1.1.1 lseed.bitcoinstats.com SRV is fairly reliable on my ISP, wonder how reliable it is for others...?
<zmnscpxj> who is willing to try?
<cdecker> Works for me xD (no surprise there I run the resolver)
<zmnscpxj> yes, I expected that, haha
<zmnscpxj> who else runs an lseed?
<cdecker> I think roasbeef has a heavily modified version
* riclas (riclas@77.7.37.188.rev.vodafone.pt) has joined
* rdymac (uid31665@gateway/web/irccloud.com/x-lijpykrklflhqpgf) has joined
<zmnscpxj> what addr?
<zmnscpxj> what URI I mean?
<cdecker> https://github.com/lightningnetwork/lnd/blob/e135047304723024dbb72cb01932ad1a3e367804/chainregistry.go#L619-L643
<cdecker> They sidestep the recursive resolver issue by forcing TCP connections
<cdecker> dig +tcp srv nodes.lightning.directory
<zmnscpxj> okay, that works directly for me
* __gotcha (~Thunderbi@plone/gotcha) has joined
<cdecker> Downside is that the DNS server learns the IP address of all nodes querying it, whereas with recursive resolving the DNS server only sees the ISP IP
<zmnscpxj> ...ah
<zmnscpxj> is it possible to proxy that somehow? It is over TCP...?
<cdecker> See the TLS over TCP querying debate for Cloudflare
<zmnscpxj> <--- is happily ignorant of basic DNS architecture and just uses Tor browser and trusts it works
<cdecker> Hehe, DNS is pretty much just decades of clutter accumulating, and different generations of ideas clashing with each other
<cdecker> Just look at the security story to ensure cryptographic integrity of records (DNSSEC et al)
<zmnscpxj> `dig @1.1.1.1 nodes.lightning.directory SRV` works, but has this scary ";; Truncated, retrying in TCP mode." comment, is that what you refer to?
<cdecker> Yep, you can go directly to TCP mode with the `+tcp` flag
<zmnscpxj> there is no way to say "do not go into TCP mode"?
<cdecker> Dunno
<cdecker> DNS over TCP is pretty new to me as well xD
<zmnscpxj> heh
<zmnscpxj> TCP mode is the reason the dns seed can see the IP of the querier, or something else?
<zmnscpxj> +notcp stll falls back to TCP, hmmm
<zmnscpxj> though `torify dig @1.1.1.1 +tcp nodes.lightning.directory SRV` works....
<zmnscpxj> cannot find a proxy flag for dig though, sigh
<zmnscpxj> Ah, +ignore
<zmnscpxj> +ignore prevents the fallback
<zmnscpxj> hmmm
<cdecker> Yep, afaik TCP mode will directly contact the authoritative DNS server and query it.
<zmnscpxj> thanks
<cdecker> Nope, apparently I'm mistaken, and recursive resolving works with TCP
<cdecker> Need to recalibrate my mental model then xD
<zmnscpxj> haha
*/

}

namespace Boss { namespace Mod {

class ConnectFinderByDns::Impl {
private:
	S::Bus& bus;

	std::vector<std::pair<std::string, std::string>> const* dnsseeds;

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
			seed.second, seed.first
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
