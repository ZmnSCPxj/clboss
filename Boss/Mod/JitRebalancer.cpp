#include"Boss/Mod/JitRebalancer.hpp"
#include"Boss/ModG/RebalanceUnmanagerProxy.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/ModG/RpcProxy.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ListpeersResult.hpp"
#include"Boss/Msg/ReleaseHtlcAccepted.hpp"
#include"Boss/Msg/RequestEarningsInfo.hpp"
#include"Boss/Msg/RequestMoveFunds.hpp"
#include"Boss/Msg/RequestPeerFromScid.hpp"
#include"Boss/Msg/RequestRebalanceBudget.hpp"
#include"Boss/Msg/ResponseEarningsInfo.hpp"
#include"Boss/Msg/ResponseMoveFunds.hpp"
#include"Boss/Msg/ResponsePeerFromScid.hpp"
#include"Boss/Msg/ResponseRebalanceBudget.hpp"
#include"Boss/Msg/ProvideHtlcAcceptedDeferrer.hpp"
#include"Boss/Msg/SolicitHtlcAcceptedDeferrer.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Ln/HtlcAccepted.hpp"
#include"Ln/NodeId.hpp"
#include"Ln/Scid.hpp"
#include"S/Bus.hpp"
#include"Stats/ReservoirSampler.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"
#include<map>

namespace {

/* Weight of an HTLC output on the commitment tx.
 * The HTLC needs to be paid for, so also factor that in our spendable
 * computation.
 */
auto constexpr htlc_weight = std::size_t(172);

}

namespace Boss { namespace Mod {

class JitRebalancer::Impl {
private:
	S::Bus& bus;
	Boss::ModG::RpcProxy rpc;

	typedef
	Boss::ModG::ReqResp< Msg::RequestRebalanceBudget
			   , Msg::ResponseRebalanceBudget
			   > RebalanceBudgetRR;
	typedef
	Boss::ModG::ReqResp< Msg::RequestMoveFunds
			   , Msg::ResponseMoveFunds
			   > MoveFundsRR;
	typedef
	Boss::ModG::ReqResp< Msg::RequestPeerFromScid
			   , Msg::ResponsePeerFromScid
			   > PeerFromScidRR;
	RebalanceBudgetRR get_budget;
	MoveFundsRR move_funds_rr;
	PeerFromScidRR peer_from_scid_rr;

	ModG::RebalanceUnmanagerProxy unmanager;

	void start() {
		bus.subscribe<Msg::SolicitHtlcAcceptedDeferrer
			     >([this
			       ](Msg::SolicitHtlcAcceptedDeferrer const&) {
			auto f = [this](Ln::HtlcAccepted::Request const& req) {
				return htlc_accepted(req);
			};
			return bus.raise(Msg::ProvideHtlcAcceptedDeferrer{
				std::move(f)
			});
		});
	}

	Ev::Io<bool>
	htlc_accepted(Ln::HtlcAccepted::Request const& req) {
		/* Is it a forward?  */
		if (!req.next_channel)
			return Ev::lift(false);

		/* Get it from the table.  */
		return peer_from_scid_rr.execute(Msg::RequestPeerFromScid{
			nullptr, req.next_channel
		}).then([ this
			, req
			](Msg::ResponsePeerFromScid const& r) {
			if (!r.peer)
				return Ev::lift(false);
			return htlc_accepted_cont( r.peer
						 , req.id
						 , req.next_amount
						 );
		});
	}
	Ev::Io<bool>
	htlc_accepted_cont( Ln::NodeId const& node
			  , std::uint64_t id
			  , Ln::Amount amount
			  ) {
		return unmanager.get_unmanaged().then([ this
						      , node
						      , amount
						      , id
						      ](std::set<Ln::NodeId> const* unmanaged) {
			if (unmanaged->count(node) != 0) {
				return Boss::log( bus, Debug
						, "JitRebalancer: HTLC %s to "
						  "unmanaged node %s, will "
						  "ignore."
						, Util::stringify(id).c_str()
						, Util::stringify(node).c_str()
						).then([]() {
					return Ev::lift(false);
				});
			}
			return Boss::concurrent( check_and_move(node, amount, id)
					       ).then([]() {
				return Ev::lift(true);
			});
		});

	}

	class Run {
	private:
		class Impl;
		std::shared_ptr<Impl> pimpl;
	public:
		Run() =delete;

		Run(S::Bus& bus, Boss::ModG::RpcProxy& rpc
		   , RebalanceBudgetRR& get_budget
		   , Ln::NodeId const& node
		   , Ln::Amount amount
		   , std::uint64_t id
		   , MoveFundsRR& move_funds_rr
		   , ModG::RebalanceUnmanagerProxy& unmanager
		   );
		Run(Run&&) =default;
		Run(Run const&) =default;
		~Run() =default;

		Ev::Io<void> execute();
	};

	Ev::Io<void>
	check_and_move( Ln::NodeId const& node
		      , Ln::Amount amount
		      , std::uint64_t id
		      ) {
		return Ev::lift().then([this, node, amount, id]() {
			auto r = Run( bus, rpc, get_budget
				    , node, amount, id
				    , move_funds_rr
				    , unmanager
				    );
			return r.execute();
		});
	}

public:
	Impl( S::Bus& bus_
	    ) : bus(bus_)
	      , rpc(bus_)
	      , get_budget(bus)
	      , move_funds_rr(bus)
	      , peer_from_scid_rr(bus)
	      , unmanager(bus)
	      { start(); }
};

/* Yes, what a messy name... */
class JitRebalancer::Impl::Run::Impl {
private:
	S::Bus& bus;
	Boss::ModG::RpcProxy& rpc;
	/* ReqResp to `Boss::Mod::RebalanceBudgeter`.  */
	RebalanceBudgetRR& get_budget;
	Ln::NodeId source;
	Ln::NodeId out_node;
	Ln::Amount amount;
	std::uint64_t id;

	/* Unilateral-close feerate.  */
	std::uint64_t feerate;
	/* Amount available on all nodes.  */
	struct ChannelInfo {
		Ln::Amount to_us;
		Ln::Amount capacity;
	};
	std::map<Ln::NodeId, ChannelInfo> available;
	/* How much should we add to the destination?  */
	Ln::Amount to_move;
	/* Up to how much to pay for *this* rebalance.  */
	Ln::Amount this_rebalance_fee;

	/* ReqResp to `Boss::Mod::FundsMover`.  */
	MoveFundsRR& move_funds_rr;
	/* Unmanager proxy.  */
	ModG::RebalanceUnmanagerProxy& unmanager;

	std::set<Ln::NodeId> const* unmanaged;

	/* Thrown to signal that we should release the HTLC.  */
	struct Continue {};
	/* Exceptions are dual to `call-with-current-continuation`,
	 * and one use of `call-with-current-continuation` is to
	 * create "early outs" without having to thread through a lot
	 * of functions.
	 * We simply use thrown objects to implement a sort of
	 * `call-with-current-continuation` to implement early outs.
	 */

	Ev::Io<void> core_execute() {
		return Ev::lift().then([this]() {
			return unmanager.get_unmanaged();
		}).then([this](std::set<Ln::NodeId> const* unmanaged_) {
			unmanaged = unmanaged_;

			/* Get the unilateral close feerate.  */
			auto parms = Json::Out()
				.start_array()
					.entry("perkw")
				.end_array()
				;
			return rpc.command("feerates", std::move(parms));
		}).then([this](Jsmn::Object res) {
			feerate = 0;
			try {
				auto data = res["perkw"];
				if (data.has("unilateral_close"))
					feerate = std::uint64_t(double(
						data["unilateral_close"]
					));
			} catch (Jsmn::TypeError const&) {
				return Boss::log( bus, Error
						, "JitRebalancer: Unexpected "
						  "result from feerates: %s"
						, Util::stringify(res).c_str()
						).then([]() {
					throw Continue();
					return Ev::lift();
				});
			}
			return Ev::lift();
		}).then([this]() {

			/* Adjust the amount by the feerate times weight
			 * plus the HTLC.  */
			amount += Ln::Amount::msat(feerate * htlc_weight);

			/* Determine the amounts available.  */
			auto parms = Json::Out::empty_object();
			return rpc.command("listpeers", std::move(parms));
		}).then([this](Jsmn::Object res) {
			try {
				auto ps = res["peers"];
				for (auto p : ps) {
					auto to_us = Ln::Amount::sat(0);
					auto capacity = Ln::Amount::sat(0);
					auto peer = Ln::NodeId(std::string(
						p["id"]
					));

					/* Skip peers in the unmanaged
					 * list.  */
					if (unmanaged->count(peer) != 0)
						continue;

					auto cs = p["channels"];
					for (auto c : cs) {
						auto state = std::string(
							c["state"]
						);
						if (state != "CHANNELD_NORMAL")
							continue;
						to_us += Ln::Amount(
							std::string(
							c["to_us_msat"]
						));
						capacity += Ln::Amount(
							std::string(
							c["total_msat"]
						));
					}

					auto& av = available[peer];
					av.to_us = to_us;
					av.capacity = capacity;
				}
			} catch (std::exception const&) {
				return Boss::log( bus, Error
						, "JitRebalancer: Unexpected "
						  "result from listpeers: %s"
						, Util::stringify(res).c_str()
						).then([]() {
					throw Continue();
					return Ev::lift();
				});
			}
			return Ev::lift();
		}).then([this]() {

			/* Check if it is available.  */
			auto it = available.find(out_node);
			if (it == available.end())
				/* Not in the map?
				 * Continue, this will fail.  */
				throw Continue();

			/* Check if the amount fits.  */
			if (amount <= it->second.to_us)
				/* Will fit, just continue.  */
				return Boss::log( bus, Debug
						, "JitRebalancer: HTLC %s "
						  "of amount %s fits in "
						  "outgoing amount %s."
						, Util::stringify(id).c_str()
						, std::string(amount).c_str()
						, std::string(it->second.to_us)
							.c_str()
						).then([]() {
					throw Continue();
					return Ev::lift();
				});

			/* Target to have the out-node have midway between
			 * the needed amount, and the total capacity of
			 * the channel, to also fund future outgoing forwards
			 * via this channel.
			 */
			auto target = (amount + it->second.capacity) / 2.0;
			to_move = target - it->second.to_us;

			return Boss::log( bus, Debug
					, "JitRebalancer: HTLC %s needs "
					  "amount %s, only %s available "
					  "at %s, want to rebalance %s."
					, Util::stringify(id).c_str()
					, std::string(amount).c_str()
					, std::string(it->second.to_us).c_str()
					, std::string(out_node).c_str()
					, std::string(to_move).c_str()
					);
		}).then([this]() {

			/* Now select a source channel.  */
			auto min_required = to_move * 1.01;
			auto sampler = Stats::ReservoirSampler<Ln::NodeId>(1);
			for (auto& e : available) {
				auto& candidate = e.first;
				auto& channel = e.second;

				/* The outgoing node is disqualified.  */
				if (candidate == out_node)
					continue;
				/* If the node does not have enough,
				 * skip it.  */
				if (channel.to_us < min_required)
					continue;

				/* Score according to how much it is
				 * balanced in our favor.  */
				sampler.add( candidate
					   , channel.to_us / channel.capacity
					   , Boss::random_engine
					   );
			}
			auto& samples = sampler.get();
			if (samples.size() == 0)
				return Boss::log( bus, Debug
						, "JitRebalancer: HTLC %s: "
						  "No candidates to get funds "
						  "from."
						, Util::stringify(id).c_str()
						).then([]() {
					throw Continue();
					return Ev::lift();
				});
			source = samples[0];

			return Boss::log( bus, Debug
					, "JitRebalancer: HTLC %s: Move %s "
					  "from %s to %s."
					, Util::stringify(id).c_str()
					, std::string(to_move).c_str()
					, std::string(source).c_str()
					, std::string(out_node).c_str()
					);
		}).then([this]() {
			/* Determine how much fee we can use for
			 * rebalancing.  */
			return get_budget.execute(Msg::RequestRebalanceBudget{
				nullptr,
			});
		}).then([this](Msg::ResponseRebalanceBudget r) {
			this_rebalance_fee = r.fee_budget;
			return move_funds();
		});
	}

	Ev::Io<void> move_funds() {
		return move_funds_rr.execute(Msg::RequestMoveFunds{
			nullptr,
			source, out_node,
			to_move, this_rebalance_fee
		}).then([](Msg::ResponseMoveFunds _) {
			/* Ignore result.  */
			return Ev::lift();
		});
	}

public:
	Impl( S::Bus& bus_
	    , Boss::ModG::RpcProxy& rpc_
	    , Boss::ModG::ReqResp< Msg::RequestRebalanceBudget
				 , Msg::ResponseRebalanceBudget
				 >& get_budget_
	    , Ln::NodeId const& out_node_
	    , Ln::Amount amount_
	    , std::uint64_t id_
	    , MoveFundsRR& move_funds_rr_
	    , ModG::RebalanceUnmanagerProxy& unmanager_
	    ) : bus(bus_), rpc(rpc_)
	      , get_budget(get_budget_)
	      , out_node(out_node_), amount(amount_), id(id_)
	      , move_funds_rr(move_funds_rr_)
	      , unmanager(unmanager_)
	      { }

	static
	Ev::Io<void> execute(std::shared_ptr<Impl> self) {
		return self->core_execute().catching<Continue
						    >([](Continue const& _) {
			return Ev::lift();
		}).catching<std::exception>([self](std::exception const& e) {
			return Boss::log( self->bus, Error
					, "JitRebalancer: Uncaught error: "
					  "%s"
					, e.what()
					);
		}).then([self]() {
			return self->bus.raise(Msg::ReleaseHtlcAccepted{
				Ln::HtlcAccepted::Response::cont(self->id)
			});
		});
	}
};
JitRebalancer::Impl::Run::Run( S::Bus& bus
			     , Boss::ModG::RpcProxy& rpc
			     , RebalanceBudgetRR& get_budget
			     , Ln::NodeId const& node
			     , Ln::Amount amount
			     , std::uint64_t id
			     , MoveFundsRR& move_funds_rr
			     , ModG::RebalanceUnmanagerProxy& unmanager
			     )
	: pimpl(std::make_shared<Impl>( bus, rpc, get_budget
				      , node, amount, id
				      , move_funds_rr
				      , unmanager
				      )) { }
Ev::Io<void> JitRebalancer::Impl::Run::execute() {
	return Impl::execute(pimpl);
}

JitRebalancer::JitRebalancer(JitRebalancer&&) =default;
JitRebalancer::~JitRebalancer() =default;

JitRebalancer::JitRebalancer(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
