#include"Boss/Mod/ChannelFinderByEarnedFee.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/PreinvestigateChannelCandidates.hpp"
#include"Boss/Msg/RequestPeerMetrics.hpp"
#include"Boss/Msg/ResponsePeerMetrics.hpp"
#include"Boss/Msg/SolicitChannelCandidates.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"
#include<algorithm>
#include<iterator>
#include<set>

namespace Boss { namespace Mod {

class ChannelFinderByEarnedFee::Impl {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;
	Ln::NodeId self_id;

	ModG::ReqResp< Msg::RequestPeerMetrics
		     , Msg::ResponsePeerMetrics
		     > get_peer_metrics_rr;

	bool running;

	/* Which peer are we going to propose?  */
	Ln::NodeId selected_peer;

	void start() {
		running = false;
		bus.subscribe< Msg::Init
			     >([this](Msg::Init const& init) {
			rpc = &init.rpc;
			self_id = init.self_id;
			return Ev::lift();
		});
		bus.subscribe<Msg::SolicitChannelCandidates
			     >([this](Msg::SolicitChannelCandidates const&) {
			if (running || !rpc)
				return Ev::lift();
			running = true;
			auto act = run().then([this]() {
				running = false;
				return Ev::lift();
			});
			return Boss::concurrent(std::move(act));
		});
	}

	Ev::Io<std::map<Ln::NodeId, Msg::PeerMetrics>>
	get_3day_peer_metrics() {
		return Ev::lift().then([this]() {
			return get_peer_metrics_rr.execute(
				Msg::RequestPeerMetrics{}
			);
		}).then([](Msg::ResponsePeerMetrics met) {
			return Ev::lift(std::move(met.day3));
		});
	}

	Ev::Io<void> run() {
		return Ev::lift().then([this]() {

			/* Find the node with the highest non-zero 3-day
			 * out-fee-msat-per-day.
			 */
			return get_3day_peer_metrics();
		}).then([this](std::map<Ln::NodeId, Msg::PeerMetrics> mets) {
			auto found = false;
			auto node = Ln::NodeId();
			auto max = double(0.0);
			for (auto const& m : mets) {
				if (m.second.out_fee_msat_per_day <= max)
					continue;
				max = m.second.out_fee_msat_per_day;
				node = m.first;
				found = true;
			}

			/* None of the nodes passed.  */
			if (!found)
				return Boss::log( bus, Info
						, "ChannelFinderByEarnedFee: "
						  "No peers with earned "
						  "outgoing fee, nothing for "
						  "us to process."
						);
			selected_peer = node;

			return Boss::log( bus, Info
					, "ChannelFinderByEarnedFee: "
					  "Proposing peers of peer %s, who "
					  "earned us %f msat/day."
					, std::string(node).c_str()
					, max
					)
			     + process_selected_peer()
			     ;
		});
	}
	Ev::Io<void> process_selected_peer() {
		return Ev::lift().then([this]() {
			auto parms = Json::Out()
				.start_object()
					.field( "source"
					      , std::string(selected_peer)
					      )
				.end_object()
				;
			return rpc->command("listchannels", std::move(parms));
		}).then([this](Jsmn::Object res) {
			/* Gather the data.  */
			auto props = std::set<Ln::NodeId>();
			try {
				auto cs = res["channels"];
				for (auto c : cs) {
					auto dest = Ln::NodeId(std::string(
						c["destination"]
					));
					if (dest == self_id)
						continue;
					props.emplace(std::move(dest));
				}
			} catch (std::exception const& ex) {
				return Boss::log( bus, Error
						, "ChannelFinderByEarnedFees: "
						  "Unexpected result from "
						  "`listchannels`: %s: %s"
						, Util::stringify(res).c_str()
						, ex.what()
						);
			}
			res = Jsmn::Object();

			/* Put in vector and shuffle.  */
			auto propv
				= std::vector<Msg::ProposeChannelCandidates>();
			std::transform( props.begin(), props.end()
				      , std::back_inserter(propv)
				      , [this](Ln::NodeId const& n) {
				return Msg::ProposeChannelCandidates{
					n, selected_peer
				};
			});
			std::shuffle( propv.begin()
				    , propv.end()
				    , Boss::random_engine
				    );

			return bus.raise(Msg::PreinvestigateChannelCandidates{
				std::move(propv), 1
			});
		});
	}

public:
	Impl() =delete;
	Impl(Impl const&) =delete;
	Impl(Impl&&) =delete;

	explicit
	Impl( S::Bus& bus_
	    ) : bus(bus_)
	      , rpc(nullptr)
	      , get_peer_metrics_rr(bus_)
	      { start(); }
};

ChannelFinderByEarnedFee::ChannelFinderByEarnedFee(ChannelFinderByEarnedFee&&)
	=default;
ChannelFinderByEarnedFee::~ChannelFinderByEarnedFee()
	=default;
ChannelFinderByEarnedFee::ChannelFinderByEarnedFee(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }
}}
