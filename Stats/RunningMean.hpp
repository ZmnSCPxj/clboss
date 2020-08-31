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
};


}

#endif /* STATS_RUNNINGMEAN_HPP */
