#include"Boss/Mod/ChannelFinderByPopularity.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Msg/Block.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ManifestCommand.hpp"
#include"Boss/Msg/ManifestNotification.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/Notification.hpp"
#include"Boss/Msg/PreinvestigateChannelCandidates.hpp"
#include"Boss/Msg/ProposeChannelCandidates.hpp"
#include"Boss/Msg/SolicitChannelCandidates.hpp"
#include"Boss/Msg/TaskCompletion.hpp"
#include"Boss/Msg/Timer10Minutes.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
#include"Ev/map.hpp"
#include"Ev/now.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<algorithm>
#include<iterator>
#include<queue>
#include<random>
#include<set>
#include<sstream>
#include<vector>

namespace {

/** max_proposals
 *
 * @brief ChannelFinderByPopularity will only
 * propose up to this number of nodes.
 */
auto const max_proposals = size_t(5);

/** min_channels_to_backoff
 *
 * @brief once we have achieved this number of
 * channels, ChannelFinderByPopularity will
 * make only one proposal.
 */
auto const min_channels_to_backoff = size_t(4);

/** min_nodes_to_process
 *
 * @brief if the number of nodes known by this node
 * is less than this, go to sleep and try again
 * later.
 */
auto const min_nodes_to_process = size_t(800);

/** seconds_after_connect
 *
 * @brief if we previously stopped looking for popular
 * nodes, and a new connection appears, how many seconds
 * should we wait for them to update us about the network?
 */
auto const seconds_after_connect = double(210);

}

namespace Boss { namespace Mod {

class ChannelFinderByPopularity::Impl {
private:
	S::Bus& bus;
	Boss::Mod::Waiter& waiter;
	void* this_ptr;
	Boss::Mod::Rpc* rpc;
	Ln::NodeId self;

	/* Set if we are already running.  */
	bool running;
	/* Set if we are waiting for a while.  */
	bool try_later;
	/* Set if we are in single-proposal mode.  */
	bool single_proposal_only;

	/* Progress tracking.  */
	double prev_time;
	std::size_t count;
	std::size_t all_nodes_count;

	void start() {
		running = false;
		try_later = false;
		single_proposal_only = false;
		bus.subscribe<Msg::Init>([this](Msg::Init const& init) {
			rpc = &init.rpc;
			self = init.self_id;
			return Ev::lift();
		});
		bus.subscribe< Msg::SolicitChannelCandidates
			     >([this](Msg::SolicitChannelCandidates const& _) {
			return on_solicit();
		});
		bus.subscribe< Msg::Block
			     >([this](Msg::Block const& _) {
			if (try_later) {
				try_later = false;
				return Boss::concurrent(Ev::lift().then([this]() {
					return solicit();
				}));
			}
			return Ev::lift();
		});
		bus.subscribe< Msg::Timer10Minutes
			     >([this](Msg::Timer10Minutes const& _) {
			if (try_later) {
				try_later = false;
				return Boss::concurrent(Ev::lift().then([this]() {
					return solicit();
				}));
			}
			return Ev::lift();
		});

		/* Also do try_later when we successfully connect.  */
		bus.subscribe< Msg::Manifestation
			     >([this](Msg::Manifestation const& _) {
			return bus.raise(Msg::ManifestNotification{
				"connect"
			}) + bus.raise(Msg::ManifestCommand{
				"clboss-findbypopularity",
				"",
				"Trigger ChannelFinderByPopularity algorithm.",
				false
			});
		});
		bus.subscribe< Msg::Notification
			     >([this](Msg::Notification const& n) {
			if (n.notification != "connect")
				return Ev::lift();
			if (!try_later)
				return Ev::lift();
			try_later = false;
			return Boss::concurrent(on_new_connect());
		});
		bus.subscribe< Msg::CommandRequest
			     >([this](Msg::CommandRequest const& r) {
			if (r.command != "clboss-findbypopularity")
				return Ev::lift();
			return bus.raise(Msg::CommandResponse{
				r.id,
				Json::Out::empty_object()
			}) + Ev::lift().then([this]() {
				return on_solicit();
			});
		});
	}

	Ev::Io<void> on_solicit() {
		if (running || !rpc || !self)
			return Ev::lift();
		running = true;
		return Boss::concurrent(Ev::lift().then([this]() {
			return solicit();
		}));
	}

	/* List of all nodes.  */
	std::queue<Ln::NodeId> all_nodes;

	struct Popular {
		Ln::NodeId node;
		std::set<Ln::NodeId> peers;
	};
	/* Number of nodes with at least one peer that passed through
	 * the A-Chao algorithm.  */
	std::size_t num_processed;
	/* A-Chao algorithm.  */
	double wsum;
	std::vector<Popular> selected;

	/* Called after a new connection while we were deferring
	 * a solicitation.  */
	Ev::Io<void> on_new_connect() {
		return Boss::log( bus, Info
				, "ChannelFinderByPopularity: "
				  "New connection, will wait %.0f seconds, "
				  "then re-find nodes."
				, seconds_after_connect
				).then([this]() {
			return waiter.wait(seconds_after_connect);
		}).then([this]() {
			return solicit();
		});
	}

	Ev::Io<void> solicit() {
		/* First check if we have at least
		 * min_channels_to_disable channels.
		 * If so, finding by popularity self-disables.
		 */
		return rpc->command( "listchannels"
				   , Json::Out()
					.start_object()
						.field("source"
						      , std::string(self)
						      )
					.end_object()
				   ).then([this](Jsmn::Object res) {
			if (!res.is_object() || !res.has("channels"))
				return self_disable();
			auto channels = res["channels"];
			if (!channels.is_array())
				return self_disable();
			auto msg = "";
			if (channels.size() >= min_channels_to_backoff) {
				single_proposal_only = true;
				msg = "We seem to have many channels, "
				      "will find one node only."
				    ;
			} else {
				single_proposal_only = false;
				msg = "Will find nodes.";
			}
			return Boss::log( bus, Debug
					, "ChannelFinderByPopularity: "
					  "%s"
					, msg
					).then([this]() {
				return continue_solicit();
			});
		});
	}

	Ev::Io<void> self_disable() {
		return Boss::log( bus, Debug
				, "ChannelFinderByPopularity: "
				  "Unexpected result of `listchannels`."
				).then([this]() {
			return signal_task_completion(true);
		});
	}

	Ev::Io<void> continue_solicit() {
		/* Now do listnodes for everything.  */
		return rpc->command( "listnodes"
				   , Json::Out::empty_object()
				   ).then([this](Jsmn::Object res){
			if (!res.is_object() || !res.has("nodes"))
				return fail_solicit("Unexpected result from listnodes.");

			auto nodes = res["nodes"];
			if (!nodes.is_array())
				return fail_solicit("Unexpected result from listnodes.");

			if (nodes.size() < min_nodes_to_process)
				return defer_solicit(nodes.size());
			all_nodes = std::queue<Ln::NodeId>();
			for (auto i = size_t(0); i < nodes.size(); ++i) {
				auto node = nodes[i];
				if (!node.is_object() || !node.has("nodeid"))
					continue;
				auto id = node["nodeid"];
				if (!id.is_string())
					continue;
				all_nodes.emplace(std::string(id));
			}
			/* Initialize the A-Chao algorithm.  */
			num_processed = 0;
			wsum = 0;
			selected.clear();
			/* Initialize progress tracking.  */
			prev_time = Ev::now();
			count = 0;
			all_nodes_count = all_nodes.size();
			/* Print details.  */
			return Boss::log( bus, Debug
					, "ChannelFinderByPopularity: "
					  "%zu nodes to be evaluated."
					, all_nodes.size()
					).then([this]() {
				return select_by_popularity();
			});
		});
	}

	Ev::Io<void> defer_solicit(size_t n) {
		try_later = true;
		return Boss::log( bus, Info
				, "ChannelFinderByPopularity: "
				  "Not enough nodes in routemap "
				  "(only %zu, need %zu), "
				  "will try again later."
				, n, min_nodes_to_process
				);
	}

	Ev::Io<void> fail_solicit(std::string const& reason) {
		return Boss::log( bus, Error
				, "ChannelFinderByPopularity: "
				  "Failed: %s"
				, reason.c_str()
				).then([this]() {
			/* Failed!  */
			return signal_task_completion(true);
		});
	}

	/* A-Chao Reservoir sampling algorithm.  */
	Ev::Io<void> select_by_popularity() {
		auto act = Ev::yield();
		if (Ev::now() - prev_time >= 5.0) {
			prev_time = Ev::now();
			act += Boss::log( bus, Info
					, "ChannelFinderByPopularity: "
					  "Progress: %zu / %zu (%f)"
					, count, all_nodes_count
					, ( double(count)
					  / double(all_nodes_count)
					  )
					);
		}

		return std::move(act).then([this]() {

		/* This is a loop.  */
		if (all_nodes.empty())
			return complete_select_by_popularity();
		/* Get channels of the current node.  */
		auto const& curr = all_nodes.front();
		return rpc->command( "listchannels"
				   , Json::Out()
					.start_object()
						.field( "source"
						      , std::string(curr)
						      )
					.end_object()
				   ).then([this](Jsmn::Object res) {
			auto entry = Popular();
			/* Now move the current node off the queue.  */
			entry.node = std::move(all_nodes.front());
			all_nodes.pop();
			++count;

			/* Now fill in the entry.  */
			if (!res.is_object() || !res.has("channels"))
				/* Skip.  */
				return select_by_popularity();
			auto channels = res["channels"];
			if (!channels.is_array())
				return select_by_popularity();
			for (auto i = size_t(0); i < channels.size(); ++i) {
				auto channel = channels[i];
				if ( !channel.is_object()
				  || !channel.has("destination")
				   )
					continue;
				auto destination = channel["destination"];
				if (!destination.is_string())
					continue;
				auto dest_s = std::string(destination);
				entry.peers.emplace(Ln::NodeId(dest_s));
			}

			if (entry.peers.size() == 0)
				return select_by_popularity();

			++num_processed;

			/* ***Finally*** determine if we should select
			 * this entry.  */
			wsum += entry.peers.size();
			/* If fewer are selected than max proposals, go extend
			 * it.  */
			auto actual_max_proposals = max_proposals;
			if (single_proposal_only)
				actual_max_proposals = 1;
			if (selected.size() < actual_max_proposals) {
				selected.emplace_back(std::move(entry));
				return select_by_popularity();
			}
			/* Otherwise see if we should replace.  */
			auto dist = std::uniform_real_distribution<double>(
				0, 1
			);
			auto p = double(entry.peers.size()) / wsum;
			auto j = dist(Boss::random_engine);
			if (j <= p) {
				/* Randomly replace an existing selection.  */
				auto dist_i = std::uniform_int_distribution<size_t>(
					0, selected.size() - 1
				);
				auto i = dist_i(Boss::random_engine);
				selected[i] = std::move(entry);
			}

			return select_by_popularity();
		});

		});
	}

	Ev::Io<void> complete_select_by_popularity() {
		/* Did the number of number of nodes processed
		 * even reach the min_nodes_to_process?
		 */
		if (num_processed < min_nodes_to_process) {
			selected.clear();
			try_later = true;
			return Boss::log( bus, Info
					, "ChannelFinderByPopularity: "
					  "Not enough nodes with >0 "
					  "channels in routemap "
					  "(only %zu, need %zu), "
					  "will try again later."
					, num_processed
					, min_nodes_to_process
					);
		}

		/* Generate a log.  */
		auto log = std::ostringstream();
		log << "ChannelFinderByPopularity: "
		    << "Random selection (by popularity): "
		    ;
		auto first = true;
		for (auto const& s : selected) {
			if (first)
				first = false;
			else
				log << ", ";
			log << s.node
			    << " (" << s.peers.size() << " peers)"
			    ;
		}
		return Boss::log( bus, Info
				, "%s", log.str().c_str()
				).then([this]() {
			/* Preinvestigate the selected nodes.  */
			return Ev::map( std::bind( &Impl::preinvestigate
						 , this
						 , std::placeholders::_1
						 )
				      , std::move(selected)
				      );
		}).then([this](std::vector<int>) {
			return signal_task_completion(false);
		});
	}

	Ev::Io<int> preinvestigate(Popular p) {
		return Boss::log( bus, Debug
				, "ChannelFinderByPopularity: "
				  "Preinvestigating peers of %s"
				, std::string(p.node).c_str()
				).then(std::bind( &Impl::preinvestigate_core
						, this
						, std::move(p)
						)).then([]() {
			return Ev::lift(0);
		});
	}

	Ev::Io<void> preinvestigate_core(Popular p) {
		/* Copy the peers and shuffle.  */
		auto peers = std::vector<Ln::NodeId>();
		std::copy( p.peers.begin(), p.peers.end()
			 , std::back_inserter(peers)
			 );
		std::shuffle(peers.begin(), peers.end(), Boss::random_engine);

		/* Create the message.  */
		auto msg = Msg::PreinvestigateChannelCandidates();
		std::transform( peers.begin(), peers.end()
			      , std::back_inserter(msg.candidates)
			      , [&p](Ln::NodeId peer) {
			return Msg::ProposeChannelCandidates{
				/* proposal = */peer, /*patron = */p.node
			};
		});
		/* For this popular node, only select one candidate.  */
		msg.max_candidates = 1;

		return bus.raise(msg);
	}

	Ev::Io<void> signal_task_completion(bool failed) {
		running = false;
		return bus.raise(Msg::TaskCompletion{
			"ChannelFinderByPopularity", this_ptr, failed
		});
	}

public:
	Impl( S::Bus& bus_
	    , Boss::Mod::Waiter& waiter_
	    , void* this_ptr_
	    ) : bus(bus_)
	      , waiter(waiter_)
	      , this_ptr(this_ptr_)
	      , rpc(nullptr)
	      , running(false)
	      { start(); }

};

ChannelFinderByPopularity::ChannelFinderByPopularity
		( S::Bus& bus
		, Boss::Mod::Waiter& waiter
		) : pimpl(Util::make_unique<Impl>(bus, waiter, this)) { }
ChannelFinderByPopularity::ChannelFinderByPopularity
		( ChannelFinderByPopularity&& o
		) : pimpl(std::move(o.pimpl)) { }
ChannelFinderByPopularity::~ChannelFinderByPopularity() { }
}}
