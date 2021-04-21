#undef NDEBUG
#include"Util/duration.hpp"
#include<assert.h>
#include<iostream>
#include<string>

namespace {

auto constexpr minute = 60.0;
auto constexpr hour = 60 * minute;
auto constexpr day = 24 * hour;
auto constexpr week = 7 * day;
auto constexpr month = 30 * day;
auto constexpr year = 365 * day;

void test_duration(double duration, char const* expected) {
	auto actual = Util::duration(duration);
	std::cout << "'" << actual << "' ?= '" << expected << "'" << std::endl;
	assert(actual == expected);
}

}

int main() {
	test_duration(0.0, "0 seconds");
	test_duration(0.1, "0.1 seconds");
	test_duration(21.345, "21 seconds");

	test_duration(2 * day + 1 * hour + 4 * minute + 2, "2 days, 1 hours, 4 minutes, 2 seconds");

	test_duration(1 * week + 2 * day, "1 weeks, 2 days");

	test_duration(2 * month + 3 * day, "2 months, 3 days");

	test_duration(2 * year + 3 * day, "2 years, 3 days");

	return 0;
}
