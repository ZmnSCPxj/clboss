#undef NDEBUG
#include"Util/date.hpp"
#include<assert.h>

int main() {
	assert(Util::date(0.0) == "UTC 1970-01-01 00:00:00.000");
	assert(Util::date(1.1) == "UTC 1970-01-01 00:00:01.100");

	return 0;
}
