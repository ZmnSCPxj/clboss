#ifndef STATS_WEIGHTEDMEDIAN_HPP
#define STATS_WEIGHTEDMEDIAN_HPP

#include"Util/BacktraceException.hpp"
#include<algorithm>
#include<functional>
#include<stdexcept>
#include<utility>
#include<vector>

namespace Stats {

/** class Stats::NoSamples
 *
 * @brief thrown when the weighted-median is
 * extracted but there are no samples.
 */
class NoSamples : public Util::BacktraceException<std::invalid_argument> {
public:
	NoSamples() : Util::BacktraceException<std::invalid_argument>("Stats::NoSamples") { }
};

/** class Stats::WeightedMedian
 *
 * @brief computes the weighted median of a set
 * of samples.
 */
template< typename Sample
	, typename Weight
	, typename CompSample = std::less<Sample>
	, typename CompWeight = std::less<Weight>
	, typename AddWeight = std::plus<Weight>
	>
class WeightedMedian {
private:
	struct Entry {
		Sample s;
		Weight w;
		Entry( Sample&& s_
		     , Weight&& w_
		     ) : s(std::move(s_))
		       , w(std::move(w_))
		       { }
	};
	std::vector<Entry> samples;
	Weight total_weight;

	CompSample cmp_s;
	CompWeight cmp_w;
	AddWeight add_w;

public:
	WeightedMedian() =default;
	WeightedMedian(WeightedMedian&&) =default;
	WeightedMedian(WeightedMedian const&) =default;
	WeightedMedian& operator=(WeightedMedian&&) =default;
	WeightedMedian& operator=(WeightedMedian const&) =default;
	~WeightedMedian() =default;

	void add(Sample s, Weight w) {
		if (samples.size() == 0)
			total_weight = w;
		else
			total_weight = add_w(std::move(total_weight), w);
		samples.emplace_back(std::move(s), std::move(w));
	}

	Sample finalize()&& {
		auto my_samples = std::move(samples);
		std::sort( my_samples.begin(), my_samples.end()
			 , [this](Entry const& a, Entry const& b) {
			return cmp_s(a.s, b.s);
		});
		auto first = true;
		auto running_weight = total_weight;
		for (auto& e : my_samples) {
			if (first) {
				running_weight = e.w;
				first = false;
			} else
				running_weight = add_w(std::move(running_weight), e.w);
			/* Add again (i.e. x2) so we do not have to do
			 * division, and implement x2 by repeated
			 * addition.
			 */
			running_weight = add_w(std::move(running_weight), e.w);
			if (cmp_w(total_weight, running_weight))
				/* Got the median.  */
				return std::move(e.s);
		}
		throw NoSamples();
	}
};

}

#endif /* !defined(STATS_WEIGHTEDMEDIAN_HPP) */
