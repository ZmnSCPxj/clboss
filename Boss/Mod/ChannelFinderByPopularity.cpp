#include"Boss/Mod/ChannelFinderByPopularity.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Block.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ProposeChannelCandidates.hpp"
#include"Boss/Msg/SolicitChannelCandidates.hpp"
#include"Boss/Msg/TaskCompletion.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
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
auto const max_proposals = size_t(4);

/** min_channels_to_disable
 *
 * @brief once we have achieved this number of
 * channels, ChannelFinderByPopularity will
 * stop making proposals.
 */
auto const min_channels_to_disable = size_t(4);

/** min_nodes_to_process
 *
 * @brief if the number of nodes known by this node
 * is less than this, go to sleep and try again
 * later.
 */
auto const min_nodes_to_process = size_t(200);

}

namespace Boss { namespace Mod {

class ChannelFinderByPopularity::Impl {
private:
	S::Bus& bus;
	void* this_ptr;
	Boss::Mod::Rpc* rpc;
	Ln::NodeId self;

	/* Set if we are already running.  */
	bool running;
	/* Set if we are waiting for a while.  */
	bool try_later;

	void start() {
		running = false;
		try_later = false;
		bus.subscribe<Msg::Init>([this](Msg::Init const& init) {
			rpc = &init.rpc;
			self = init.self_id;
			return Ev::lift();
		});
		bus.subscribe< Msg::SolicitChannelCandidates
			     >([this](Msg::SolicitChannelCandidates const& _) {
			if (running || !rpc || !self)
				return Ev::lift();
			running = true;
			return Boss::concurrent(Ev::lift().then([this]() {
				return solicit();
			}));
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
	}

	/* List of all nodes.  */
	std::queue<Ln::NodeId> all_nodes;

	struct Popular {
		Ln::NodeId node;
		std::set<Ln::NodeId> peers;
	};
	/* A-Chao algorithm.  */
	double wsum;
	std::vector<Popular> selected;

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
			if (channels.size() >= min_channels_to_disable)
				return self_disable();
			return Boss::log( bus, Debug
					, "ChannelFinderByPopularity: "
					  "Will find nodes."
					).then([this]() {
				return continue_solicit();
			});
		});
	}

	Ev::Io<void> self_disable() {
		return Boss::log( bus, Debug
				, "ChannelFinderByPopularity: "
				  "Voluntarily not operating, "
				  "we seem to have many channels."
				).then([this]() {
			/* Do not signal failed, since we successfully
			 * decided not to do anything.  */
			return signal_task_completion(false);
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
			wsum = 0;
			selected.clear();
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
		return Ev::yield().then([this]() {

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

			/* ***Finally*** determine if we should select
			 * this entry.  */
			wsum += entry.peers.size();
			/* If fewer are selected than max proposals, go extend
			 * it.  */
			if (selected.size() < max_proposals) {
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
			return try_connect();
		});
	}

	std::vector<Popular>::const_iterator sel_it;
	std::vector<Ln::NodeId> peers;
	std::vector<Ln::NodeId>::const_iterator peer_it;
	bool made_proposal;

	Ev::Io<void> try_connect() {
		/* Now we try to connect to each of the peers
		 * of each of the selected nodes.
		 * If we manage to connect to one of the peers,
		 * we propose that peer and set the popular node
		 * as the patron.
		 *
		 * We go over each of the selected nodes in the
		 * loop_selected, then shuffle the peers of that
		 * node, then go over each of the peers in the
		 * loop_peers.
		 */

		/* FIXME: Ideally we would parallelize over each entry
		 * in the loop_selected.
		 * We can do that later.  */

		/* Start the loop_selected.  */
		sel_it = selected.begin();
		/* We have not made any proposals yet.  */
		made_proposal = false;
		
		return loop_selected();
	}

	Ev::Io<void> loop_selected() {
		return Ev::yield().then([this]() {

		if (sel_it == selected.end())
			return completed_solicit();

		/* Load peers and shuffle.  */
		peers.clear();
		std::copy( sel_it->peers.begin(), sel_it->peers.end()
			 , std::back_inserter(peers)
			 );
		std::shuffle(peers.begin(), peers.end(), Boss::random_engine);

		/* Start looping over peers.  */
		peer_it = peers.begin();
		return loop_peers();

		});
	}

	Ev::Io<void> loop_peers() {
		return Ev::yield().then([this]() {

		if (peer_it == peers.end()) {
			++sel_it;
			return loop_selected();
		}
		/* FIXME: We should use the Connector object via its
		 * messages.  */
		return rpc->command( "connect"
				   , Json::Out()
					.start_object()
						.field( "id"
						      , std::string(*peer_it)
						      )
					.end_object()
				   ).then([this](Jsmn::Object _) {
			/* Success.  */
			return Ev::lift(true);
		}).catching<RpcError>([this](RpcError const& _) {
			return Ev::lift(false);
		}).then([this](bool success) {
			/* If it succeeded, send proposal and exit loop.  */
			if (success) {
				return bus.raise(Msg::ProposeChannelCandidates{
					*peer_it, sel_it->node
				}).then([this]() {
					made_proposal = true;
					peer_it = peers.end();
					return Ev::lift();
				});
			} else {
				++peer_it;
				return Ev::lift();
			}
		}).then([this]() {
			return loop_peers();
		});

		});
	}


	Ev::Io<void> completed_solicit() {
		return Ev::yield().then([this]() {
			if (made_proposal)
				return Boss::log( bus, Info
						, "ChannelFinderByPopularity: "
						  "Proposed some nodes to "
						  "channel to, finished."
						);
			else
				return Boss::log( bus, Warn
						, "ChannelFinderByPopularity: "
						  "No nodes were proposed."
						);
		}).then([this]() {
			return signal_task_completion(!made_proposal);
		});
	}

	Ev::Io<void> signal_task_completion(bool failed) {
		running = false;
		return bus.raise(Msg::TaskCompletion{
			"ChannelFinderByPopularity", this_ptr, failed
		});
	}

public:
	Impl( S::Bus& bus_
	    , void* this_ptr_
	    ) : bus(bus_)
	      , this_ptr(this_ptr_)
	      , rpc(nullptr)
	      , running(false)
	      { start(); }

};

ChannelFinderByPopularity::ChannelFinderByPopularity
		( S::Bus& bus
		) : pimpl(Util::make_unique<Impl>(bus, this)) { }
ChannelFinderByPopularity::ChannelFinderByPopularity
		( ChannelFinderByPopularity&& o
		) : pimpl(std::move(o.pimpl)) { }
ChannelFinderByPopularity::~ChannelFinderByPopularity() { }
}}
