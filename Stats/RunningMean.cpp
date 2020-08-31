#include"Stats/RunningMean.hpp"
#include"Util/Str.hpp"
#include<cstdlib>

namespace {

union UD {
	std::uint64_t u;
	double d;
};

}

namespace Stats {

std::ostream& operator<<(std::ostream& os, RunningMean& m) {
	union UD dat;
	dat.d = m.mean;

	os << std::hex << dat.u << " " << m.samples << " ";

	return os;
}

std::istream& operator>>(std::istream& is, RunningMean& m) {
	union UD dat;

	is >> std::hex >> dat.u >> m.samples;
	m.mean = dat.d;

	return is;
}

}
