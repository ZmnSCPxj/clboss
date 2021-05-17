#include"Boss/Mod/PeerComplaintsDesk/Exempter.hpp"
#include"Boss/Mod/PeerComplaintsDesk/Unmanager.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/RequestPeerMetrics.hpp"
#include"Boss/Msg/ResponsePeerMetrics.hpp"
#include"Ev/Io.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"
#include<algorithm>
#include<functional>
#include<math.h>
#include<vector>

namespace {

/* Channels below this age are exempted.
 * The `min_age_description` is a human-readable description of the
 * time frame below.
 */
auto const min_age = double(2 * 24 * 60 * 60);
auto const min_age_description = std::string("two days");
/* Top percentile of fee earners.
 * i.e. the top `high_earner_percentile` percent of all peers are
 * exempted from complaints.
 */
auto const high_earner_percentile = double(40);

}

namespace Boss { namespace Mod { namespace PeerComplaintsDesk {

class Exempter::Impl {
private:
	ModG::ReqResp< Msg::RequestPeerMetrics
		     , Msg::ResponsePeerMetrics
		     > metrics_rr;
	Unmanager& unmanager;

public:
	Impl() =delete;
	Impl(Impl const&) =delete;
	Impl(Impl&&) =delete;

	explicit
	Impl( S::Bus& bus
	    , Unmanager& unmanager_
	    ) : metrics_rr( bus
			  , [](Msg::RequestPeerMetrics& m, void* p) {
				m.requester = p;
			    }
			  , [](Msg::ResponsePeerMetrics& m) {
				return m.requester;
			    }
			  )
	      , unmanager(unmanager_)
	      { }

	Ev::Io<std::map<Ln::NodeId, std::string>>
	get_exemptions() {
		return Ev::lift().then([this]() {
			return metrics_rr.execute(Msg::RequestPeerMetrics{});
		}).then([this](Msg::ResponsePeerMetrics all_metrics) {
			auto const& metrics = all_metrics.day3;

			auto rv = std::map<Ln::NodeId, std::string>();

			/* Function to add exemptions to particular
			 * nodes.  */
			auto add_exempt = [&rv]( Ln::NodeId const& nid
					       , std::string const& why
					       ) {
				auto it = rv.find(nid);
				if (it == rv.end())
					rv[nid] = why;
				else
					it->second += std::string("; ") + why;
			};
			/* Gather data.  */
			auto has_nonzero_in = false;
			auto has_nonzero_out = false;
			for (auto& m : metrics) {
				if (m.second.age < min_age)
					add_exempt( m.first
						  , std::string("channel is ")
						  + "younger than "
						  + min_age_description
						  );
				if (m.second.in_fee_msat_per_day > 0)
					has_nonzero_in = true;
				if (m.second.out_fee_msat_per_day > 0)
					has_nonzero_out = true;
			}
			/* Generic function to find top earners.  */
			auto exempt_top_earner = [ &add_exempt
						 , &metrics
						 ]( std::string const& what
						  , std::function<
						double(Ln::NodeId const&)
						    > scorer) {
				/* Nothing to do?  */
				if (metrics.size() == 0)
					return;

				auto full_reason
					= std::string("node is in top ")
					+ Util::stringify(
						high_earner_percentile
					  )
					+ std::string("% of highest ")
					+ what
					+ " earners"
					;

				typedef std::pair< Ln::NodeId
						 , double
						 > PeerEntry;

				/* Extract peers and their scores.  */
				auto peers = std::vector<PeerEntry>();
				for (auto& m : metrics)
					peers.push_back(std::make_pair(
						m.first,
						scorer(m.first)
					));
				/* Sort from highest-scorers to lowest.  */
				std::sort( peers.begin(), peers.end()
					 , []( PeerEntry const& a
					     , PeerEntry const& b
					     ) {
					return b.second < a.second;
				});

				/* Find the the index of the percentile point.  */
				auto ind = std::size_t(
					ceil( ( high_earner_percentile
					      * peers.size()
					      )
					    / 100.0
					    )
				);
				/* Could happen if high_earner_percentile >= 50%
				 * and peers.size() == 1.  */
				if (ind == peers.size())
					ind = peers.size() - 1;
				/* Find the minimum score.  */
				auto min_score = peers[ind].second;
				if (min_score == 0.0) {
					/* Only exempt non-zero scorers.
					 * This means that if the top
					 * percentile includes nodes with
					 * 0 score, the 0-scorers are still
					 * not exempted.
					 */
					for (auto& p : peers) {
						if (p.second <= 0.0)
							continue;
						add_exempt( p.first
							  , full_reason
							  );
					}
				} else {
					/* Otherwise exempt only
					 * those who have min
					 * score or higher.
					 */
					for (auto& p : peers) {
						if (p.second < min_score)
							continue;
						add_exempt( p.first
							  , full_reason
							  );
					}
				}
			};
			if (has_nonzero_in)
				exempt_top_earner( "incoming fee"
						 , [&metrics
						   ](Ln::NodeId const& n) {
					auto it = metrics.find(n);
					auto& inf = it->second;
					return inf.in_fee_msat_per_day;
				});
			if (has_nonzero_out)
				exempt_top_earner( "outgoing fee"
						 , [&metrics
						   ](Ln::NodeId const& n) {
					auto it = metrics.find(n);
					auto& inf = it->second;
					return inf.out_fee_msat_per_day;
				});

			/* Add exemptions for unmanaged nodes.  */
			auto const& unmanaged = unmanager.get_unmanaged();
			for (auto const& n : unmanaged)
				add_exempt( n
					  , "node has `close` unmanagement tag"
					  );

			return Ev::lift(std::move(rv));
		});
	}
};

Exempter::Exempter(Exempter&&) =default;
Exempter::~Exempter() =default;

Exempter::Exempter(S::Bus& bus, Unmanager& unmanager)
	: pimpl(Util::make_unique<Impl>(bus, unmanager)) { }

Ev::Io<std::map<Ln::NodeId, std::string>>
Exempter::get_exemptions() {
	return pimpl->get_exemptions();
}

}}}
