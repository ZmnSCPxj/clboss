#undef NDEBUG
#include"Util/format.hpp"
#include<assert.h>

int main() {
	assert(Util::format("foo") == "foo");
	assert(Util::format("foo %d", 42) == "foo 42");
	assert(Util::format("foo %d %0.1f", 42, double(0.0)) == "foo 42 0.0");
	assert(Util::format("foo %s batz", "bar") == "foo bar batz");

	return 0;
}
