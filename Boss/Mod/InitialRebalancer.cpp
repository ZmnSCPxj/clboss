#include"Boss/Mod/InitialRebalancer.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/ListpeersResult.hpp"
#include"Boss/Msg/RequestMoveFunds.hpp"
#include"Boss/Msg/ResponseMoveFunds.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Stats/ReservoirSampler.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"
#include<map>
#include<vector>

namespace {

/* If the spendable amount exceeds this percent of the channel total,
 * this code triggers.
 */
auto constexpr spendable_percent = double(80.0);
/* Limit on rebalance fee.  */
auto const rebalance_fee = Ln::Amount::sat(15);

}

namespace Boss { namespace Mod {

class InitialRebalancer::Impl {
private:
	S::Bus& bus;

	void start() {
		bus.subscribe<Msg::ListpeersResult
			     >([this](Msg::ListpeersResult const& m) {
			return run(m.peers);
		});
	}

	class Run {
	private:
		class Impl;
		std::shared_ptr<Impl> pimpl;

	public:
		Run() =delete;

		Run(Run&&) =default;
		~Run() =default;

		explicit
		Run(S::Bus& bus, Jsmn::Object const& peers);
		Ev::Io<void> run();
	};

	Ev::Io<void>
	run(Jsmn::Object const& peers) {
		return Boss::concurrent(Run(bus, peers).run());
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	explicit
	Impl(S::Bus& bus_) : bus(bus_) { start(); }
};

class InitialRebalancer::Impl::Run::Impl {
private:
	S::Bus& bus;
	Jsmn::Object peers;

	/* Data about a peer.  */
	struct Info {
		Ln::Amount spendable;
		Ln::Amount receivable;
		Ln::Amount total;
	};
	std::map<Ln::NodeId, Info> info;
	/* Plan to move.  */
	std::vector<std::pair<Ln::NodeId, Ln::NodeId>> plan;

	/* Interface to funds mover.  */
	ModG::ReqResp< Msg::RequestMoveFunds
		     , Msg::ResponseMoveFunds
		     > move_rr;

	Ev::Io<void> core_run() {
		return Ev::lift().then([this]() {
			try {
				for ( auto i = std::size_t(0)
				    ; i < peers.size()
				    ; ++i
				    ) {
					auto spendable = Ln::Amount::sat(0);
					auto receivable = Ln::Amount::sat(0);
					auto total = Ln::Amount::sat(0);

					auto p = peers[i];
					auto id = Ln::NodeId(std::string(
						p["id"]
					));

					auto cs = p["channels"];
					for ( auto j = std::size_t(0)
					    ; j < cs.size()
					    ; ++j
					    ) {
						auto c = cs[j];
						auto priv = bool(
							c["private"]
						);
						if (priv)
							continue;
						auto state = std::string(
							c["state"]
						);
						if (state != "CHANNELD_NORMAL")
							continue;
						if ( !c.has("total_msat")
						  || !c.has("spendable_msat")
						  || !c.has("receivable_msat")
						   )
							continue;
						spendable += Ln::Amount(
							std::string(
							c["spendable_msat"]
						));
						receivable += Ln::Amount(
							std::string(
							c["receivable_msat"]
						));
						total += Ln::Amount(
							std::string(
							c["total_msat"]
						));
					}

					if (total == Ln::Amount::sat(0))
						continue;
					info[id].spendable = spendable;
					info[id].receivable = receivable;
					info[id].total = total;
				}
			} catch (std::exception const& _) {
				return Boss::log( bus, Error
						, "InitialRebalancer: "
						  "Unexpected result from "
						  "listpeers: %s"
						, Util::stringify(peers)
							.c_str()
						);
			}
			return plan_move();
		});
	}
	Ev::Io<void> plan_move() {
		/* Gather sources and destinations.  */
		auto sources = std::vector<Ln::NodeId>();
		auto destinations = std::map<Ln::NodeId, Info>();
		for ( auto it = info.begin(), next = info.begin()
		    ; it != info.end()
		    ; it = next
		    ) {
			next = it;
			++next;

			auto& info = it->second;
			if ( (info.spendable / info.total)
			  >= (spendable_percent / 100.0)
			   )
				sources.push_back(it->first);
			else
				destinations.insert(*it);
		}

		/* Nothing to do.  */
		if (sources.empty())
			return Ev::lift();

		for (auto& s : sources) {
			if (destinations.empty())
				break;

			auto sampler = Stats::ReservoirSampler<Ln::NodeId>(1);
			for (auto& d : destinations) {
				auto& info = d.second;
				sampler.add( d.first
					   , info.receivable / info.total
					   , Boss::random_engine
					   );
			}

			auto dest = std::move(sampler).finalize()[0];
			plan.push_back(std::make_pair(s, dest));
		}

		return execute_plan();
	}

	Ev::Io<void> execute_plan() {
		auto act = Ev::lift();
		for (auto& p : plan) {
			auto source = p.first;
			auto destination = p.second;
			auto s_info = info[source];
			auto d_info = info[destination];

			auto max_send = s_info.spendable / 2.0;

			auto max_dest = d_info.total * ( spendable_percent
						       / 100.0
						       );
			auto max_receive = max_dest - d_info.spendable;

			auto amount = max_send;
			if (amount > max_receive)
				amount = max_receive;

			act += Boss::concurrent(move_funds( source
							  , destination
							  , amount
							  ));
		}
		return act;
	}

	Ev::Io<void> move_funds( Ln::NodeId const& source
			       , Ln::NodeId const& destination
			       , Ln::Amount amount
			       ) {
		return move_rr.execute(Msg::RequestMoveFunds{
			nullptr, source, destination, amount, rebalance_fee
		}).then([](Msg::ResponseMoveFunds _) {
			return Ev::lift();
		});
	}

public:
	Impl( S::Bus& bus_
	    , Jsmn::Object const& peers_
	    ) : bus(bus_), peers(peers_)
	      , move_rr( bus_
		       , [](Msg::RequestMoveFunds& msg, void* p) {
				msg.requester = p;
			 }
		       , [](Msg::ResponseMoveFunds& msg) {
				return msg.requester;
			 }
		       )
	      { }
	static
	Ev::Io<void> run(std::shared_ptr<Impl> self) {
		return self->core_run().then([self]() {
			return Ev::lift();
		});
	}
};

InitialRebalancer::Impl::Run::Run( S::Bus& bus
				 , Jsmn::Object const& peers
				 ) : pimpl(std::make_shared<Impl>(bus, peers))
				   { }
Ev::Io<void> InitialRebalancer::Impl::Run::run() {
	return Impl::run(pimpl);
}

InitialRebalancer::InitialRebalancer(InitialRebalancer&&) =default;
InitialRebalancer::~InitialRebalancer() =default;

InitialRebalancer::InitialRebalancer(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
