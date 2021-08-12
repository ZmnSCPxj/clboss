#include"Boss/Mod/PeerJudge/Algo.hpp"
#include"Json/Out.hpp"
#include"Stats/WeightedMedian.hpp"
#include"Util/format.hpp"
#include"Util/make_unique.hpp"
#include<algorithm>
#include<assert.h>

namespace Boss { namespace Mod { namespace PeerJudge {

class Algo::Impl {
private:
	std::vector<Info> infos;
	double feerate;
	std::uint64_t reopen_weight;

	Ln::Amount reopen_cost;

	struct ScoreData {
		Ln::NodeId node;
		Ln::Amount total;
		Ln::Amount earned;

		/* Score data.  */
		double earned_per_size;

		/* Judgment data.  */
		Ln::Amount expected_earnings_if_reopened;
		Ln::Amount expected_improvement_after_reopen_cost;
	};
	std::vector<ScoreData> scores;
	std::vector<ScoreData>::iterator median;
	struct ScoreDataItComp {
		bool operator()( std::vector<ScoreData>::iterator a
			       , std::vector<ScoreData>::iterator b
			       ) const {
			return a->earned_per_size < b->earned_per_size;
		}
	};
	std::map<Ln::NodeId, std::string> to_close;

	Json::Out get_inputs_status() const {
		auto rv = Json::Out();
		auto arr = rv.start_array();
		for (auto const& i : scores) {
			arr.start_object()
				.field("node", std::string(i.node))
				.field("earned", std::string(i.earned))
				.field("total", std::string(i.total))
				.field("earned_over_total", i.earned_per_size)
				.field( "earned_over_total_ppm"
				      , i.earned_per_size * 1000000.0
				      )
			.end_object()
			;
		}
		arr.end_array();
		return rv;
	}
	Json::Out get_candidates_status() const {
		auto rv = Json::Out();
		auto arr = rv.start_array();
		for (auto it = median + 1; it != scores.end(); ++it) {
			arr.start_object()
				.field("node", std::string(it->node))
				.field( "expected_earnings_if_reopened"
				      , std::string(it->expected_earnings_if_reopened)
				      )
				.field( "actual_earned"
				      , std::string(it->earned)
				      )
				.field( "expected_improvement_after_reopen_cost"
				      , std::string(it->expected_improvement_after_reopen_cost)
				      )
				.field( "close"
				      , bool(to_close.find(it->node) != to_close.end())
				      )
			.end_object()
			;
		}
		arr.end_array();
		return rv;
	}

public:
	Impl( std::vector<Info> infos_
	    , double feerate_
	    , std::uint64_t reopen_weight_
	    ) : infos(std::move(infos_))
	      , feerate(feerate_)
	      , reopen_weight(reopen_weight_)
	      { }

	Json::Out get_status() const {
		return Json::Out()
			.start_object()
				.field("feerate_perkw", feerate)
				.field("reopen_weight", reopen_weight)
				.field("channels", get_inputs_status())
				.field("reopen_cost", std::string(reopen_cost))
				.field("weighted_median", median->earned_per_size)
				.field( "weighted_median_ppm"
				      , median->earned_per_size * 1000000.0
				      )
				.field("to_close", get_candidates_status())
			.end_object()
			;
	}
	std::map<Ln::NodeId, std::string> get_close() const {
		return to_close;
	}
	void judge_peers() {
		assert(infos.size() != 0);

		/* Reset data.  */
		to_close.clear();
		scores.clear();

		/* Compute cost to reopen.  */
		reopen_cost = Ln::Amount::sat((feerate * reopen_weight) / 1000.0);

		/* Create the scores and sort by score from
		 * highest to lowest.  */
		for (auto const& i : infos) {
			scores.emplace_back(ScoreData{
				i.id,
				i.total_normal,
				i.earned,
				double(i.earned.to_msat()) / double(i.total_normal.to_msat())
			});
		}
		std::sort( scores.begin(), scores.end()
			 , [](ScoreData const& a, ScoreData const& b) {
			return a.earned_per_size > b.earned_per_size;
		});

		assert(scores.size() == infos.size());

		/* Now fill in WeightedMedian.  */
		auto wm = Stats::WeightedMedian< std::vector<ScoreData>::iterator
					       , Ln::Amount
					       , ScoreDataItComp
					       >();
		for (auto it = scores.begin(); it != scores.end(); ++it) {
			wm.add(it, it->total);
		}
		median = std::move(wm).finalize();

		auto median_per_total = median->earned_per_size;

		/* Now actually judge.  */
		for (auto it = median + 1; it != scores.end(); ++it) {
			/* We expect the earnings to on average be the
			 * the median earnings-per-channel-size,
			 * times the size of this particular channel.
			 */
			it->expected_earnings_if_reopened = median_per_total * it->total;
			/* Take advantage of the fact that Ln::Amount saturates
			 * to 0 on subtraction.  */
			it->expected_improvement_after_reopen_cost =
				( it->expected_earnings_if_reopened
				- it->earned
				- reopen_cost
				);
			if ( it->expected_improvement_after_reopen_cost
			  != Ln::Amount::sat(0)
			   ) {
				auto reason = Util::format( "Expected (median) "
							    "earning rate: %f (%f ppm); "
							    "expected earnings given "
							    "size: %s; "
							    "actual earnings: %s; "
							    "reopen cost: %s"
							  , median_per_total
							  , median_per_total * 1000000.0
							  , std::string(
						it->expected_earnings_if_reopened
							    ).c_str()
							  , std::string(
								it->earned
							    ).c_str()
							  , std::string(
								reopen_cost
							    ).c_str()
							  );
				to_close[it->node] = reason;
			}
		}
	}
};

Algo::Algo() : pimpl() { }
Algo::~Algo() =default;

Json::Out Algo::get_status() const {
	if (!pimpl)
		return Json::Out::direct("no data");
	return pimpl->get_status();
}
std::map<Ln::NodeId, std::string> Algo::get_close() const {
	if (!pimpl)
		return std::map<Ln::NodeId, std::string>();
	return pimpl->get_close();
}
void Algo::judge_peers( std::vector<Info> infos
		      , double feerate
		      , std::uint64_t reopen_weight
		      ) {
	if (infos.size() == 0) {
		pimpl = nullptr;
		return;
	}
	pimpl = Util::make_unique<Impl>( std::move(infos)
				       , feerate
				       , reopen_weight
				       );
	pimpl->judge_peers();
}

}}}
