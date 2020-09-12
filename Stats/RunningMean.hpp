#ifndef STATS_RUNNINGMEAN_HPP
#define STATS_RUNNINGMEAN_HPP

#include<cstdlib>
#include<iostream>

namespace Stats {

class RunningMean {
private:
	double mean;
	std::size_t samples;

public:
	RunningMean() : samples(0) { }

	double get() const { return mean; }
	void sample(double x) {
		if (samples == 0) {
			samples = 1;
			mean = x;
			return;
		}

		mean = (mean * samples / (samples + 1))
		     + (x / (samples + 1))
		     ;
		++samples;
	}

	friend
	std::istream& operator>>(std::istream&, RunningMean&);
	friend
	std::ostream& operator<<(std::ostream&, RunningMean&);

	/* Allows you to save a RunningMean somewhere else.  */
	struct Memo {
		double mean;
		std::size_t samples;
	};
	Memo get_memo() const {
		auto memo = Memo();
		memo.mean = mean;
		memo.samples = samples;
		return memo;
	}
	RunningMean( Memo const& memo
		   ) : mean(memo.mean)
		     , samples(memo.samples)
		     { }
};


}

#endif /* STATS_RUNNINGMEAN_HPP */
