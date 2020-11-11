#include"Boss/Mod/ChannelFinderByDistance.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/PreinvestigateChannelCandidates.hpp"
#include"Boss/Msg/SolicitChannelCandidates.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
#include"Ev/now.hpp"
#include"Ev/yield.hpp"
#include"Graph/Dijkstra.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Ln/Amount.hpp"
#include"S/Bus.hpp"
#include"Stats/ReservoirSampler.hpp"
#include"Util/stringify.hpp"
#include<algorithm>
#include<math.h>
#include<memory>
#include<queue>

namespace {

/* Amount to simulate passing through channels to
 * measure their cost.
 */
auto const reference_amount = Ln::Amount::sat(10000);
/* Conversion rate of delay to millisatoshi.
 */
auto const msat_per_block = double(1.0);
/* Maximum fee per hop.
 */
auto const max_fee = Ln::Amount::sat(50); /* 0.5% of reference_amount */

/* Maximum number to give to preinvestigation.  */
auto const max_preinvestigate = std::size_t(40);
/* Maximum number to tell preinvestigation to pass.  */
auto const max_candidates = std::size_t(3);

/* At initial startup, we can have channels, but all of them
 * are inactive because not connected yet.
 * If we have any channels and all are inactive, defer by
 * these number of seconds.
 */
auto const initial_startup_delay = double(30.0);

}

namespace Boss { namespace Mod {

class ChannelFinderByDistance::Run : public std::enable_shared_from_this<Run> {
private:
	S::Bus& bus;
	Boss::Mod::Rpc& rpc;
	Boss::Mod::Waiter& waiter;
	Ln::NodeId self_id;

	typedef 
	Graph::Dijkstra< Ln::NodeId
		       , Ln::Amount
		       > Dijkstra;
	typedef Dijkstra::TreeNode TreeNode;
	typedef Dijkstra::Result DijkstraResult;

	Dijkstra djk;
	/* Resulting navigation guide.  */
	DijkstraResult nav;

	/* Progress reporting.  */
	double prev_time;
	std::size_t progress_count;

	/* Used to extact leaf nodes in the Dijkstra run.  */
	std::queue<TreeNode const*> extract_q;
	/* Actual leaf nodes.  */
	std::vector<TreeNode const*> leaves;

	Run( S::Bus& bus_
	   , Boss::Mod::Rpc& rpc_
	   , Boss::Mod::Waiter& waiter_
	   , Ln::NodeId const& self_id_
	   ) : bus(bus_)
	     , rpc(rpc_)
	     , waiter(waiter_)
	     , self_id(self_id_)
	     , djk(self_id_, Ln::Amount::sat(0))
	     { }

public:
	Run() =delete;
	Run(Run&&) =delete;
	Run(Run const&) =delete;

	static
	std::shared_ptr<Run>
	create( S::Bus& bus
	      , Boss::Mod::Rpc& rpc
	      , Boss::Mod::Waiter& waiter
	      , Ln::NodeId const& self_id
	      ) {
		return std::shared_ptr<Run>(
			new Run(bus, rpc, waiter, self_id)
		);
	}

	Ev::Io<void> run() {
		auto self = shared_from_this();
		return self->core_run().then([self]() {
			return Ev::lift();
		});
	}

private:
	Ev::Io<void> core_run() {
		return Ev::lift().then([this]() {
			/* Check first if local channels exist and are
			 * inactive.
			 */
			auto parms = Json::Out()
				.start_object()
					.field("source", std::string(self_id))
				.end_object()
				;
			return rpc.command("listchannels", std::move(parms)
					  );
		}).then([this](Jsmn::Object res) {
			auto empty = true;
			auto any_active = false;
			try {
				auto cs = res["channels"];
				empty = cs.size() == 0;
				for (auto c : cs) {
					if (c["active"]) {
						any_active = true;
						break;
					}
				}
			} catch (Jsmn::TypeError const&) {
				return Boss::log( bus, Error
						, "ChannelFinderByDistance: "
						  "Starting listchannels "
						  "gave unexpected result: "
						  "%s"
						, Util::stringify(res)
							.c_str()
						);
			}
			if (empty)
				return Boss::log( bus, Info
						, "ChannelFinderByDistance: "
						  "We are not on network."
						);

			auto act = Ev::lift();
			if (!any_active) {
				act += Boss::log( bus, Info
						, "ChannelFinderByDistance: "
						  "Local channels not yet "
						  "active, will wait %.0f "
						  "seconds."
						, initial_startup_delay
						);
				act += waiter.wait(initial_startup_delay);
			}
			act += start_processing();
			return act;
		});
	}

	Ev::Io<void> start_processing() {
		return Ev::lift().then([this]() {
			prev_time = Ev::now();
			progress_count = 0;
			return Boss::log( bus, Debug
					, "ChannelFinderByDistance: "
					  "Starting Dijkstra."
					)
			     + loop()
			     + find_leaves()
			     + analyze()
			     ;
		});
	}
	Ev::Io<void> loop() {
		auto act = Ev::yield();
		if (Ev::now() - prev_time >= 5.0) {
			prev_time = Ev::now();
			act += Boss::log( bus, Info
					, "ChannelFinderByDistance: "
					  "Dijkstra progress: "
					  "%zu nodes scanned."
					, progress_count
					);
		}

		return std::move(act).then([this]() {
			auto n = djk.current();
			if (!n)
				return Boss::log( bus, Debug
						, "ChannelFinderByDistance: "
						  "Dijkstra complete."
						);
			return step(*n);
		});
	}
	Ev::Io<void> step(Ln::NodeId const& n) {
		++progress_count;
		auto parms = Json::Out()
			.start_object()
				.field("source", std::string(n))
			.end_object()
			;
		return rpc.command("listchannels", std::move(parms)
				  ).then([this](Jsmn::Object res) {
			try {
				auto cs = res["channels"];
				for (auto c : cs) {
					if (!c["active"])
						continue;
					/* Neighbor.  */
					auto n = Ln::NodeId(std::string(
						c["destination"]
					));
					/* (b)ase and (p)roportional fees.  */
					auto b = Ln::Amount::msat(double(
						c["base_fee_millisatoshi"]
					));
					auto p = double(
						c["fee_per_millionth"]
					);
					/* (d)elay in blocks.  */
					auto d = double(
						c["delay"]
					);
					auto cost = b
						  + ( reference_amount
						    * ( p
						      / 1000000.0
						      ))
						  + Ln::Amount::msat(
							msat_per_block * d
						    )
						  ;
					if (cost > max_fee)
						continue;
					djk.neighbor(n, cost);
				}
			} catch (Jsmn::TypeError const&) {
				return Boss::log( bus, Error
						, "ChannelFinderByDistance: "
						  "Unexpected listchannels "
						  "result: %s"
						, Util::stringify(res)
							.c_str()
						);
			}

			djk.end_neighbors();
			return loop();
		});
	}

	Ev::Io<void> find_leaves() {
		return Ev::lift().then([this]() {
			prev_time = Ev::now();
			progress_count = 0;

			nav = std::move(djk).finalize();
			auto n = nav[self_id].get();
			extract_q.push(n);
			return find_leaves_loop();
		});
	}
	Ev::Io<void> find_leaves_loop() {
		auto act = Ev::yield();
		if (Ev::now() - prev_time >= 5.0) {
			prev_time = Ev::now();
			act += Boss::log( bus, Info
					, "ChannelFinderByDistance: "
					  "Leaf progress: %zu nodes scanned."
					, progress_count
					);
		}

		return std::move(act).then([this]() {
			if (extract_q.empty())
				return Boss::log( bus, Debug
						, "ChannelFinderByDistance: "
						  "Found %zu leaves."
						, leaves.size()
						);

			auto n = extract_q.front();
			extract_q.pop();
			++progress_count;
			/* Is it a leaf?  */
			if (n->children.empty())
				leaves.push_back(n);
			else
				/* Push the children.  */
				for (auto c : n->children)
					extract_q.push(c);

			return find_leaves_loop();
		});
	}

	Ev::Io<void> analyze() {
		struct Entry {
			Ln::NodeId proposal;
			Ln::NodeId patron;
			Ln::Amount cost;
		};
		return Ev::lift().then([this]() {
			/* Use reservoir sampling, with the cost to
			 * reach the node as the weight, to select
			 * some number of leaves.
			 */
			auto rsv = Stats::ReservoirSampler<Entry>(
				max_preinvestigate
			);
			for (auto n : leaves) {
				/* Ignore self and direct channels of self.  */
				if (!n->parent)
					continue;
				if (*n->parent->data.first == self_id)
					continue;
				auto cost = n->data.second;
				auto e = Entry{ *n->data.first
					      , *n->parent->data.first
					      , cost
					      };
				rsv.add( std::move(e)
				       , cost.to_btc()
				       , Boss::random_engine
				       );
			}
			auto entries = std::move(rsv).finalize();
			/* Sort by cost from highest to lowest.  */
			std::sort( entries.begin(), entries.end()
				 , [](Entry const& a, Entry const& b) {
				return b.cost < a.cost;
			});

			/* Build a report.  */
			auto os = std::ostringstream();
			auto first = true;
			auto count = std::size_t(0);
			for (auto const& e : entries) {
				if (first) {
					os << "Preinvestigating: ";
					first = false;
				} else
					os << ", ";
				if (count >= 8) {
					os << " ... "
					   << entries.size() - count
					   << " more"
					   ;
					break;
				}
				++count;

				os << e.proposal << "("
				   << e.cost
				   << ")"
				    ;
			}
			if (first)
				os << "No candidate leaves.";
			auto act = Boss::log( bus, Info
					    , "ChannelFinderByDistance: "
					      "%s"
					    , os.str().c_str()
					    );
			/* Create the message.  */
			auto cands = std::vector<Msg::ProposeChannelCandidates
						>(entries.size());
			std::transform( entries.begin(), entries.end()
				      , cands.begin()
				      , [](Entry const& e) {
				return Msg::ProposeChannelCandidates{
					std::move(e.proposal),
					std::move(e.patron)
				};
			});
			auto msg = Msg::PreinvestigateChannelCandidates{
				std::move(cands), max_candidates
			};

			act += bus.raise(msg);

			return act;
		});
	}
};

void ChannelFinderByDistance::start() {
	bus.subscribe<Msg::Init
		     >([this](Msg::Init const& init) {
		rpc = &init.rpc;
		self_id = init.self_id;
		return Ev::lift();
	});
	bus.subscribe<Msg::SolicitChannelCandidates
		     >([this](Msg::SolicitChannelCandidates const& _) {
		if (!rpc)
			return Ev::lift();
		if (running)
			return Ev::lift();

		running = true;
		auto run = Run::create(bus, *rpc, waiter, self_id);
		return Boss::concurrent(run->run().then([this]() {
			running = false;
			return Ev::lift();
		}));
	});
}

}}
