#include"Boss/Mod/ChannelCandidateInvestigator/Main.hpp"
#include"Boss/Mod/ChannelCreator/Carpenter.hpp"
#include"Boss/Mod/ChannelCreator/Manager.hpp"
#include"Boss/Mod/ChannelCreator/Planner.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/RequestChannelCreation.hpp"
#include"Boss/Msg/SolicitChannelCandidates.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Ln/Amount.hpp"
#include"Net/IPAddrOrOnion.hpp"
#include"Net/IPBinnerBySubnet.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<algorithm>
#include<assert.h>
#include<sstream>

namespace {

/* Minimum and maximum channel size.  */
auto const min_amount = Ln::Amount::btc(0.005);
auto const max_amount = Ln::Amount::sat(16777215);
/* If it is difficult to fit the amount among multiple
 * proposals, how much should we target leaving until
 * next time?
 */
auto const min_remaining = Ln::Amount::btc(0.0105);

/* If all the entries in the plan are 0, the plan is empty.  */
bool plan_is_empty(std::map<Ln::NodeId, Ln::Amount> const& plan) {
	return std::all_of( plan.begin(), plan.end()
			  , [](std::pair<Ln::NodeId, Ln::Amount> const& e) {
		return e.second == Ln::Amount::sat(0);
	});
}

Ev::Io<void> report_proposals( S::Bus& bus, char const* prefix
			     , std::vector< std::pair<Ln::NodeId, Ln::NodeId>
					  > const& proposals
			     ) {
	auto os = std::ostringstream();

	auto first = true;
	for (auto& p : proposals) {
		if (first)
			first = false;
		else
			os << ", ";
		os << p.first;
	}
	return Boss::log( bus, Boss::Debug
			, "ChannelCreator: %s: %s"
			, prefix
			, os.str().c_str()
			);
}

}

namespace Boss { namespace Mod { namespace ChannelCreator {

void Manager::start() {
	bus.subscribe<Msg::Init
		     >([this](Msg::Init const& init) {
		rpc = &init.rpc;
		self = init.self_id;
		reprioritizer = Util::make_unique<Reprioritizer>
			( init.signer
			, Util::make_unique<Net::IPBinnerBySubnet>()
			, [this](Ln::NodeId n) { return get_node_addr(n); }
			, [this]() { return get_peers(); }
			);
		return Ev::lift();
	});
	bus.subscribe<Msg::RequestChannelCreation
		     >([this](Msg::RequestChannelCreation const& rcc) {
		if (!rpc)
			return Ev::lift();
		return Boss::concurrent(
			on_request_channel_creation(rcc.amount)
		);
	});
}

Ev::Io<void>
Manager::on_request_channel_creation(Ln::Amount amt) {
	auto num_chans = std::make_shared<std::size_t>();
	auto plan = std::make_shared<std::map<Ln::NodeId, Ln::Amount>>();

	return Ev::lift().then([this]() {
		return Boss::log( bus, Debug
				, "ChannelCreator: Triggered."
				);
	}).then([this]() {
		return rpc->command("getinfo"
				   , Json::Out::empty_object()
				   );
	}).then([this, num_chans](Jsmn::Object info) {
		*num_chans = (double)info["num_pending_channels"]
			   + (double)info["num_active_channels"]
			   ;
		return investigator.get_channel_candidates();
	}).then([this](std::vector<std::pair<Ln::NodeId, Ln::NodeId>> proposals) {
		return reprioritize(std::move(proposals));
	}).then([ this
		, num_chans
		, amt
		](std::vector<std::pair<Ln::NodeId, Ln::NodeId>> proposals) {
		/* Construct the dowser function.  */
		auto dowser_func = [this]( Ln::NodeId proposal
					 , Ln::NodeId patron
					 ) {
			auto amount = std::make_shared<Ln::Amount>();

			return Ev::lift().then([this, proposal, patron]() {
				return dowser.execute(Msg::RequestDowser{
					nullptr, proposal, patron
				});
			}).then([this
				, amount
				, proposal
				, patron
				](Msg::ResponseDowser resp) {
				*amount = resp.amount;
				return Boss::log( bus, Debug
						, "ChannelCreator: "
						  "Propose %s to %s "
						  "(patron %s)"
						, std::string(*amount).c_str()
						, std::string(proposal).c_str()
						, std::string(patron).c_str()
						);
			}).then([amount]() {
				return Ev::lift(*amount);
			});
		};
		auto planner = Planner( std::move(dowser_func)
				      , amt
				      , std::move(proposals)
				      , *num_chans
				      , min_amount
				      , max_amount
				      , min_remaining
				      );
		return std::move(planner).run();
	}).then([plan](std::map<Ln::NodeId, Ln::Amount> n_plan) {
		*plan = n_plan;

		return Ev::yield();
	}).then([this, plan]() {

		if (plan_is_empty(*plan)) {
			return Boss::log( bus, Info
					, "ChannelCreator: Insufficient "
					  "channel candidates, will solicit "
					  "more."
					).then([this]() {
				return bus.raise(
					Msg::SolicitChannelCandidates()
				);
			});
		}

		auto report = std::ostringstream();
		auto first = true;
		for (auto const& p : *plan) {
			if (p.second == Ln::Amount::sat(0))
				continue;
			if (first)
				first = false;
			else
				report << ", ";
			report << p.first << ": " << p.second;
		}

		return Boss::log( bus, Info
				, "ChannelCreator: %s"
				, report.str().c_str()
				);
	}).then([this, plan]() {
		/* Carpenter is responsible for disseminating 0-amount
		 * candidates as failures to create channels.
		 * So `plan_is_empty` will still cause this to be called,
		 * in case the plan has any 0-amount entries.
		 * Thus, we need to separately check that the plan is truly
		 * empty here, else the Carpenter assert will trigger.
		 */
		if (plan->empty())
			return Ev::lift();
		return carpenter.construct(std::move(*plan));
	});
}
Ev::Io<std::unique_ptr<Net::IPAddrOrOnion>>
Manager::get_node_addr(Ln::NodeId n) {
	assert(rpc);
	return Ev::lift().then([this, n]() {
		return rpc->command("listnodes"
				   , Json::Out()
					.start_object()
						.field("id", std::string(n))
					.end_object()
				   );
	}).then([this](Jsmn::Object res) {
		auto rv = Net::IPAddrOrOnion();
		try {
			auto nodes = res["nodes"];
			/* Node not known?  */
			if (nodes.length() == 0)
				return Ev::lift(std::unique_ptr<Net::IPAddrOrOnion>());
			auto node = nodes[0];
			auto addrs = node["addresses"];
			/* No addresses known for node?  */
			if (addrs.length() == 0)
				return Ev::lift(std::unique_ptr<Net::IPAddrOrOnion>());
			/* Report first address.  */
			auto addr_j = addrs[0];
			auto addr_s = std::string(addr_j["address"]);
			rv = Net::IPAddrOrOnion(addr_s);
		} catch (...) {
			return Boss::log( bus, Error
					, "ChannelCreator: Unexpected result from "
					  "listnodes: %s"
					, res.direct_text().c_str()
					).then([]() {
				return Ev::lift(std::unique_ptr<Net::IPAddrOrOnion>());
			});
		}
		return Ev::lift(Util::make_unique<Net::IPAddrOrOnion>(std::move(rv)));
	});
}
Ev::Io<std::vector<Ln::NodeId>>
Manager::get_peers() {
	assert(rpc);
	return Ev::lift().then([this]() {
		return rpc->command("listpeers", Json::Out::empty_object());
	}).then([this](Jsmn::Object res) {
		auto rv = std::vector<Ln::NodeId>();
		try {
			auto peers = res["peers"];
			for (auto peer : peers) {
				auto id_j = peer["id"];
				auto id_s = std::string(id_j);
				auto id = Ln::NodeId(id_s);
				rv.push_back(std::move(id));
			}
		} catch (...) {
			return Boss::log( bus, Error
					, "ChannelCreator: Unexpected result from "
					  "listpeers: %s"
					, res.direct_text().c_str()
					).then([rv]() {
				return Ev::lift(rv);
			});
		}
		return Ev::lift(std::move(rv));
	});
}
Ev::Io<std::vector<std::pair<Ln::NodeId, Ln::NodeId>>>
Manager::reprioritize(std::vector<std::pair<Ln::NodeId, Ln::NodeId>> proposals_v) {
	auto proposals = std::make_shared<std::vector<std::pair<Ln::NodeId, Ln::NodeId>>>
		(std::move(proposals_v));
	return Ev::lift().then([this, proposals]() {
		return report_proposals( bus, "Proposals from ChannelCandidateInvestigator"
				       , *proposals
				       );
	}).then([this, proposals]() {
		return reprioritizer->reprioritize(std::move(*proposals));
	}).then([this, proposals](std::vector< std::pair<Ln::NodeId, Ln::NodeId>
					     > n_proposals) {
		*proposals = std::move(n_proposals);
		return report_proposals( bus, "After reprioritization from IP binning"
				       , *proposals
				       );
	}).then([proposals]() {
		return Ev::lift(std::move(*proposals));
	});
}

}}}
