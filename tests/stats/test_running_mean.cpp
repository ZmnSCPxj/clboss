#undef NDEBUG
#include"Stats/RunningMean.cpp"
#include<assert.h>
#include<sstream>

int main() {
	auto m1 = Stats::RunningMean();

	m1.sample(1.0);
	assert(m1.get() == 1.0);
	m1.sample(2.0);
	assert(m1.get() == 1.5);
	m1.sample(3.0);
	assert(m1.get() == 2.0);
	m1.sample(4.0);
	assert(m1.get() == 2.5);
	m1.sample(1.0);
	assert(m1.get() == 2.2);

	/* Check serialize/deserialize code.  */
	auto ss = std::stringstream();
	ss << m1;
	auto m2 = Stats::RunningMean();
	ss >> m2;
	m1.sample(4.0); m2.sample(4.0);
	assert(m1.get() == m2.get());
	m1.sample(1.0); m2.sample(1.0);
	assert(m1.get() == m2.get());

	return 0;
}
