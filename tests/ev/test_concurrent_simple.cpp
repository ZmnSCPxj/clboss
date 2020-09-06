#undef NDEBUG
#include<assert.h>
#include"Ev/Io.hpp"
#include"Ev/concurrent.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"

int main() {
	auto flag1 = bool(false);
	auto flag2 = bool(false);
	auto ec = Ev::start(Ev::yield().then([&]() {
		return Ev::concurrent(Ev::yield().then([&]() {
			flag1 = true;
			return Ev::lift();
		}));
	}).then([&]() {
		/* Concurrent task not started yet.  */
		assert(!flag1);
		flag2 = true;
		return Ev::lift(0);
	}));
	assert(flag1);
	assert(flag2);

	return ec;
}
