#include"Boss/Mod/FeeModderBySize.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ChannelCreation.hpp"
#include"Boss/Msg/ChannelDestruction.hpp"
#include"Boss/Msg/ProvideChannelFeeModifier.hpp"
#include"Boss/Msg/SolicitChannelFeeModifier.hpp"
#include"Boss/Msg/TimerRandomDaily.hpp"
#include"Boss/Shutdown.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/map.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Ln/Amount.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"
#include<algorithm>
#include<assert.h>
#include<map>
#include<math.h>
#include<set>
#include<sstream>

/*
<zmnscpxj__> the"median of peers of peer" is a reasonable starting point IMO
<darosior> FWIW i don't think it is :)
<zmnscpxj__> how so
<zmnscpxj__> ?
<darosior> From my own experience setting really high fees when you are a consequent router is the more reasonnable
<darosior> Like way more than all your peers
<zmnscpxj__> "consequent router" means what?
<darosior> I mean i've been testing different economic strategies on a ~70-100 channels node for months
<zmnscpxj__> okay, so what does "consequent router" mean? larger than your competitors?
<darosior> Ah, i meant like big enough to do some stats..

ZmnSCPxj: Not sure what "big enough to do some stats" here is but I
assume for now we are talking about large nodes with high amounts of
liquidity and channels.
Thus, this module works by getting our size, in terms of total channel
size, and comparing ourselves with the competitors.
*/

namespace {

/* Given the number of nodes that are lower-capacity than us,
 * the number who are higher-capacity, and the total number of
 * competitors, what is the multiplier on the feerates we
 * should use.
 */
double
calculate_multiplier( std::size_t worse
		    , std::size_t better
		    , std::size_t total
		    ) {
	assert(worse + better <= total);
	if (total == 0)
		return 1.0;

	/* Form our position as a number between 0.0 to 1.0.  */
	auto pos = double(worse) / double(worse + better);

	auto x = double();
	if (worse < better) {
		/* Remap position from 0 to 0.5 as 0.0 to 1.0.  */
		pos *= 2;
		/* Map position of 0 to sqrt(0.5) and 0.5 as 1.0.  */
		auto static const base = sqrt(0.5);
		x = base - pos * base + pos * 1.0;
	} else {
		/* The more total peers, the lower our fee multiplier limit.  */
		auto log_inv_total = log(1.0 / double(total));
		auto lim = 16.0 + log_inv_total;
		/* This would require the peer to have ridiculous numbers
		 * of peers.
		 */
		if (lim < 2)
			lim = 2.0;

		/* Remap position from 0.5 to 1.0 as 0.0 to 1.0.  */
		pos = pos * 2 - 1.0;
		/* Map 0.0 to sqrt(1.0) and 1.0 as sqrt(lim).  */
		auto static constexpr base = 1.0;
		x = base - pos * base + pos * sqrt(lim);
	}

	return x * x;
}

}

namespace Boss { namespace Mod {

class FeeModderBySize::Impl {
private:
	S::Bus& bus;

	Boss::Mod::Rpc* rpc;
	Ln::NodeId self_id;

	/* Total channel capacity of our node.  */
	std::unique_ptr<Ln::Amount> my_capacity;
	bool computing_my_capacity;
	std::vector<std::function<void()>> my_capacity_waiters;

	/* Cache of multipliers.  */
	std::map<Ln::NodeId, double> multipliers;

	bool shutdown;

	void start() {
		bus.subscribe<Msg::Init
			     >([this](Msg::Init const& init) {
			rpc = &init.rpc;
			self_id = init.self_id;
			return Ev::lift();
		});
		bus.subscribe<Msg::SolicitChannelFeeModifier
			     >([this
			       ](Msg::SolicitChannelFeeModifier const& _) {
			auto f = [this]( Ln::NodeId peer
				       , std::uint32_t base
				       , std::uint32_t proportional
				       ) {
				return get_cached_multiplier(peer);
			};
			return bus.raise(Msg::ProvideChannelFeeModifier{
				std::move(f)
			});
		});
		/* Reset our cached data daily to force recomputation.  */
		bus.subscribe<Msg::TimerRandomDaily
			     >([this](Msg::TimerRandomDaily const& _) {
			return reset();
		});
		/* Channel creation and destruction events can change
		 * our channel capacity, so we should reset our cached
		 * data as well.
		 */
		bus.subscribe<Msg::ChannelCreation
			     >([this](Msg::ChannelCreation const& _) {
			return reset();
		});
		bus.subscribe<Msg::ChannelDestruction
			     >([this](Msg::ChannelDestruction const& _) {
			return reset();
		});

		bus.subscribe<Shutdown>([this](Shutdown const& _) {
			shutdown = true;
			return awaken_waiters();
		});
	}

	Ev::Io<void> reset() {
		multipliers.clear();

		if (computing_my_capacity && !my_capacity) {
			/* A long-running process is *still* computing
			 * our capacity.
			 * Do not touch the other data.
			 */
			return Ev::lift();
		}

		computing_my_capacity = false;
		my_capacity = nullptr;
		return awaken_waiters();
	}

	Ev::Io<void> wait_for_rpc() {
		return Ev::yield().then([this]() {
			if (rpc)
				return Ev::lift();
			if (shutdown)
				throw Shutdown();
			return wait_for_rpc();
		});
	}

	/* Makes sure my_capacity is non-null.  */
	Ev::Io<void> ensure_my_capacity() {
		return Ev::lift().then([this]() {
			if (shutdown)
				throw Shutdown();
			if (my_capacity)
				return Ev::lift();
			if (computing_my_capacity)
				/* This can still awaken with
				 * my_capacity==null, e.g. if
				 * triggered by reset or shutdown.
				 */
				return wait_for_my_capacity()
				     + ensure_my_capacity()
				     ;
			computing_my_capacity = true;

			return wait_for_rpc().then([this]() {
				return compute_my_capacity();
			}).then([this](Ln::Amount n_capacity) {
				my_capacity = Util::make_unique<Ln::Amount>(
					n_capacity
				);

				return awaken_waiters();
			});
		});
	}
	Ev::Io<void> wait_for_my_capacity() {
		return Ev::Io<void>([this
				    ]( std::function<void()> pass
				     , std::function<void(std::exception_ptr)
						    > fail
				     ) {
			if (my_capacity)
				return pass();
			my_capacity_waiters.emplace_back(std::move(pass));
		}) + Ev::yield();
	}
	Ev::Io<void> awaken_waiters() {
		return Ev::Io<void>([this
				    ]( std::function<void()> pass
				     , std::function<void(std::exception_ptr)
						    > fail
				     ) {
			auto waiters = std::move(my_capacity_waiters);
			my_capacity_waiters.clear();
			for (auto const& w : waiters)
				w();
			pass();
		});
	}

	Ev::Io<Ln::Amount> compute_my_capacity() {
		return compute_capacity(self_id);
	}

	Ev::Io<Ln::Amount> compute_capacity(Ln::NodeId const& node) {
		auto parms = Json::Out()
			.start_object()
				.field("source", std::string(node))
			.end_object()
			;
		return rpc->command( "listchannels"
				   , std::move(parms)
				   ).then([this](Jsmn::Object res) {
			auto running_cap = Ln::Amount::sat(0);
			try {
				auto cs = res["channels"];
				for (auto c : cs) {
					/* Only care about public and
					 * active channels.
					 */
					if (!c["public"])
						continue;
					if (!c["active"])
						continue;
					running_cap += Ln::Amount(std::string(
						c["amount_msat"]
					));
				}
			} catch (std::runtime_error const& _) {
				return Boss::log( bus, Error
						, "FeeModderBySize: "
						  "compute_capacity: "
						  "Unexpected result from "
						  "listchannels: %s"
						, Util::stringify(res)
							.c_str()
						).then([running_cap]() {
					return Ev::lift(running_cap);
				});
			}
			return Ev::lift(running_cap);
		});
	}

	Ev::Io<double> get_cached_multiplier(Ln::NodeId peer) {
		auto it = multipliers.find(peer);
		if (it != multipliers.end())
			return Ev::lift(it->second);
		return compute_multiplier(peer
					 ).then([ this
						, peer
						](double mult) {
			multipliers[peer] = mult;
			return Ev::lift(mult);
		});
	}
	Ev::Io<double> compute_multiplier(Ln::NodeId peer) {
		auto caps = std::make_shared<std::vector<Ln::Amount>>();

		return wait_for_rpc().then([this, peer]() {
			return get_competitors(peer);
		}).then([this](std::vector<Ln::NodeId> competitors) {
			auto f = [this](Ln::NodeId n) {
				return compute_capacity(n);
			};
			return Ev::map( std::move(f)
				      , std::move(competitors)
				      );
		}).then([this, caps](std::vector<Ln::Amount> n_caps) {
			/* We were suspended while talking to RPC,
			 * so make sure to ensure capacity *after*,
			 * in case of a race condition where the
			 * capacity was erased.
			 */
			*caps = std::move(n_caps);
			return ensure_my_capacity();
		}).then([this, caps, peer]() {
			auto os = std::ostringstream();

			auto worse = std::size_t(0);
			auto better = std::size_t(0);
			auto total = std::size_t(0);
			for (auto const& c : *caps) {
				if (c < *my_capacity)
					++worse;
				else if (*my_capacity < c)
					++better;
				++total;
			}
			os << "Peer " << peer << " has "
			   << total << " other peers, "
			   << worse << " of which have less capacity than us, "
			   << better << " have more."
			    ;

			auto multiplier = calculate_multiplier( worse
							      , better
							      , total
							      );
			os << "  Multiplier: " << multiplier;

			return Boss::log( bus, Info
					, "FeeModderBySize: %s"
					, os.str().c_str()
					).then([multiplier]() {
				return Ev::lift(multiplier);
			});
		});
	}

	/* Get the competitors we have in the channel with the peer.
	 * These are basically the peers of that peer other than
	 * ourself.
	 */
	Ev::Io<std::vector<Ln::NodeId>>
	get_competitors(Ln::NodeId const& peer) {
		auto parms = Json::Out()
			.start_object()
				.field("source", std::string(peer))
			.end_object()
			;
		return rpc->command( "listchannels"
				   , std::move(parms)
				   ).then([this](Jsmn::Object res) {
			auto rv = std::set<Ln::NodeId>();
			try {
				auto cs = res["channels"];
				for (auto c : cs) {
					auto n = Ln::NodeId(std::string(
						c["destination"]
					));
					if (n == self_id)
						continue;
					rv.insert(n);
				}
			} catch (std::exception const&) {
				return Boss::log( bus, Error
						, "FeeModderBySize: "
						  "get_competitors: "
						  "Unexpected result from "
						  "listchannels: %s"
						, Util::stringify(res)
							.c_str()
						).then([rv]() {
					return Ev::lift(rv);
				});
			}
			return Ev::lift(std::move(rv));
		}).then([](std::set<Ln::NodeId> competitors) {
			/* Convert to vector.  */
			auto rv = std::vector<Ln::NodeId>(competitors.size());
			std::copy( competitors.begin(), competitors.end()
				 , rv.begin()
				 );
			return Ev::lift(std::move(rv));
		});
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	explicit
	Impl(S::Bus& bus_) : bus(bus_)
			   , rpc(nullptr)
			   , computing_my_capacity(false)
			   , shutdown(false)
			   {
		start();
	}
};

FeeModderBySize::FeeModderBySize(FeeModderBySize&&) =default;
FeeModderBySize::~FeeModderBySize() =default;

FeeModderBySize::FeeModderBySize(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
