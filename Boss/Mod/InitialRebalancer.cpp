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
#include<sstream>
#include<vector>

namespace {

/* If the spendable amount exceeds this percent of the channel total,
 * this code triggers.
 */
auto constexpr spendable_percent = double(80.0);
/* Gap to prevent destinations from hitting the spendable_percent.  */
auto constexpr dest_gap_percent = double(5.0);
/* Limit on rebalance fee.  */
auto const min_rebalance_fee = Ln::Amount::sat(5);
auto constexpr rebalance_fee_percent = double(0.5);

}

namespace Boss { namespace Mod {

class InitialRebalancer::Impl {
private:
	S::Bus& bus;
	/* Interface to funds mover.  */
	typedef 
	ModG::ReqResp< Msg::RequestMoveFunds
		     , Msg::ResponseMoveFunds
		     > MoveRR;
	MoveRR move_rr;

	void start() {
		bus.subscribe<Msg::ListpeersResult
			     >([this](Msg::ListpeersResult const& m) {
			/* If this is the initial startup, then we might not
			 * be connected to the peers involved yet, so better
			 * to wait and let it "simmer" a bit.
			 */
			if (m.initial)
				return Ev::lift();
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
		Run(S::Bus& bus, Jsmn::Object const& peers, MoveRR& move_rr);
		Ev::Io<void> run();
	};

	Ev::Io<void>
	run(Jsmn::Object const& peers) {
		return Boss::concurrent(Run(bus, peers, move_rr).run());
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	explicit
	Impl( S::Bus& bus_
	    ) : bus(bus_)
	      , move_rr( bus_
		       , [](Msg::RequestMoveFunds& msg, void* p) {
				msg.requester = p;
			 }
		       , [](Msg::ResponseMoveFunds& msg) {
				return msg.requester;
			 }
		       )
	      { start(); }
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
		     >& move_rr;

	Ev::Io<void> core_run() {
		return Ev::lift().then([this]() {
			try {
				for (auto p : peers) {
					auto spendable = Ln::Amount::sat(0);
					auto receivable = Ln::Amount::sat(0);
					auto total = Ln::Amount::sat(0);

					auto id = Ln::NodeId(std::string(
						p["id"]
					));

					auto cs = p["channels"];
					for (auto c : cs) {
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
						compute_spendable( spendable
								 , receivable
								 , total
								 , c
								 );
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
	void compute_spendable( Ln::Amount& a_spendable
			      , Ln::Amount& a_receivable
			      , Ln::Amount& a_total
			      , Jsmn::Object const& c
			      ) {
		if ( !c.has("to_us_msat")
		  || !c.has("total_msat")
		  || !c.has("htlcs")
		   )
			return;
		/* FIXME: Handle reserves.  */
		auto to_us = Ln::Amount(std::string(c["to_us_msat"]));
		auto total = Ln::Amount(std::string(c["total_msat"]));
		auto to_them = total - to_us;
		for (auto h : c["htlcs"])
			to_them -= Ln::Amount(std::string(
				h["amount_msat"]
			));
		a_spendable += to_us;
		a_receivable += to_them;
		a_total += total;
	}
	Ev::Io<void> plan_move() {
		auto msg = std::ostringstream();
		auto first = true;
		/* Gather sources and destinations.  */
		auto sources = std::vector<Ln::NodeId>();
		auto destinations = std::map<Ln::NodeId, Info>();
		for ( auto it = info.begin(), next = info.begin()
		    ; it != info.end()
		    ; it = next
		    ) {
			next = it;
			++next;

			if (first)
				first = false;
			else
				msg << ", ";

			auto& info = it->second;
			auto peer_spendable_percent = ( info.spendable
						      / info.total
						      )
						    * 100.0
						    ;
			msg << it->first << ": "
			    << peer_spendable_percent << "% "
			    ;
			if (peer_spendable_percent >= spendable_percent) {
				sources.push_back(it->first);
				msg << "(source)";
			} else if ( peer_spendable_percent
				 >= (spendable_percent - dest_gap_percent)
				  ) {
				msg << "(neutral)";
			} else {
				destinations.insert(*it);
				msg << "(destination)";
			}
		}
		auto act = Ev::lift();
		if (!first)
			act += Boss::log( bus, Debug
					, "InitialRebalancer: %s"
					, msg.str().c_str()
					);

		/* Nothing to do.  */
		if (sources.empty())
			return act;

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

		return std::move(act) + execute_plan();
	}

	Ev::Io<void> execute_plan() {
		auto act = Ev::lift();
		for (auto& p : plan) {
			auto source = p.first;
			auto destination = p.second;
			auto s_info = info[source];
			auto d_info = info[destination];

			auto max_send = s_info.spendable / 2.0;

			auto max_dest = d_info.total * ( ( spendable_percent
							 - dest_gap_percent
							 )
						       / 100.0
						       );
			auto max_receive = max_dest - d_info.spendable;

			auto amount = max_send;
			if (amount > max_receive)
				amount = max_receive;

			if (amount == Ln::Amount::sat(0))
				continue;

			act += Boss::log( bus, Debug
					, "InitialRebalancer: %s --> %s --> %s"
					, std::string(source).c_str()
					, std::string(amount).c_str()
					, std::string(destination).c_str()
					);
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
		auto this_rebalance_fee = amount
					* (rebalance_fee_percent / 100.0)
					;
		if (this_rebalance_fee < min_rebalance_fee)
			this_rebalance_fee = min_rebalance_fee;
		return move_rr.execute(Msg::RequestMoveFunds{
			nullptr, source, destination, amount,
			this_rebalance_fee
		}).then([](Msg::ResponseMoveFunds _) {
			return Ev::lift();
		});
	}

public:
	Impl( S::Bus& bus_
	    , Jsmn::Object const& peers_
	    , MoveRR& move_rr_
	    ) : bus(bus_), peers(peers_)
	      , move_rr(move_rr_)
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
				 , MoveRR& move_rr
				 ) : pimpl(std::make_shared<Impl>(bus, peers, move_rr))
				   { }
Ev::Io<void> InitialRebalancer::Impl::Run::run() {
	return Impl::run(pimpl);
}

InitialRebalancer::InitialRebalancer(InitialRebalancer&&) =default;
InitialRebalancer::~InitialRebalancer() =default;

InitialRebalancer::InitialRebalancer(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
