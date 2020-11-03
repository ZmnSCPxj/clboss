#include"Boss/Mod/ChannelFinderByListpays.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ListpeersAnalyzedResult.hpp"
#include"Boss/Msg/ProposePatronlessChannelCandidate.hpp"
#include"Boss/Msg/SolicitChannelCandidates.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
#include"Ev/foreach.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include"Stats/ReservoirSampler.hpp"
#include"Util/stringify.hpp"
#include<algorithm>
#include<iterator>
#include<map>

namespace {

/* Maximum number of items to propose.  */
auto const max_proposals = std::size_t(2);

}

namespace Boss { namespace Mod {

void ChannelFinderByListpays::start() {
	bus.subscribe<Msg::Init
		     >([this](Msg::Init const& init) {
		rpc = &init.rpc;
		return Ev::lift();
	});
	bus.subscribe<Msg::ListpeersAnalyzedResult
		     >([this](Msg::ListpeersAnalyzedResult const& r) {
		channels.clear();
		std::set_union( r.connected_channeled.begin()
			      , r.connected_channeled.end()
			      , r.disconnected_channeled.begin()
			      , r.disconnected_channeled.end()
			      , std::inserter(channels, channels.begin())
			      );
		return Ev::lift();
	});
	bus.subscribe<Msg::SolicitChannelCandidates
		     >([this](Msg::SolicitChannelCandidates const& m) {
		if (!rpc)
			return Ev::lift();
		if (running)
			return Ev::lift();

		running = true;
		auto act = rpc->command( "listpays", Json::Out::empty_object()
				       ).then([this](Jsmn::Object res) {
			auto payees = std::map<Ln::NodeId, std::size_t>();
			try {
				auto pays = res["pays"];
				for ( auto i = std::size_t(0)
				    ; i < pays.size()
				    ; ++i
				    ) {
					auto pay = pays[i];
					if (!pay.has("destination"))
						continue;
					auto payee = Ln::NodeId(std::string(
						pay["destination"]
					));

					/* Do we already have a channel with
					 * that payee?  */
					auto cit = channels.find(payee);
					if (cit != channels.end())
						continue;

					/* Track number of payments.  */
					auto pit = payees.find(payee);
					if (pit == payees.end())
						payees[payee] = 1;
					else
						++pit->second;
				}
			} catch (Jsmn::TypeError const&) {
				return Boss::log( bus, Error
						, "ChannelFinderByListpays: "
						  "Unexpected result from "
						  "listpays: %s"
						, Util::stringify(res).c_str()
						);
			}

			/* Select by number of payments made.  */
			auto smp = Stats::ReservoirSampler<Ln::NodeId>(
				max_proposals
			);
			for (auto const& p : payees)
				smp.add( p.first, double(p.second)
				       , Boss::random_engine
				       );
			auto proposals = std::move(smp).finalize();

			/* Generate report.  */
			auto os = std::ostringstream();
			auto first = true;
			for (auto const& p : proposals) {
				if (first) {
					os << "Proposing: ";
					first = false;
				} else
					os << ", ";
				os << p
				   << "(" << std::dec << payees[p] << ")"
				    ;
			}
			if (first)
				os << "No proposals.";
			auto act = Boss::log( bus, Debug
					    , "ChannelFinderByListpays: %s"
					    , os.str().c_str()
					    );
			auto f = [this](Ln::NodeId id) {
				return bus.raise(Msg::ProposePatronlessChannelCandidate{
					std::move(id)
				});
			};
			act += Ev::foreach(std::move(f), std::move(proposals));

			return act;
		}).then([this]() {
			running = false;
			return Ev::lift();
		});
		return Boss::concurrent(act);
	});
}

}}
