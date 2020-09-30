#undef NDEBUG
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include<assert.h>

int main() {
	auto flag1 = false;
	auto flag2 = false;
	auto flag3 = false;
	auto flag4 = false;

	auto act1 = Ev::lift();
	act1 += Ev::lift().then([&]() {
		assert(!flag1);
		flag1 = true;
		return Ev::lift();
	});

	auto act2 = Ev::lift().then([&]() {
		assert(flag1);
		assert(!flag2);
		flag2 = true;
		return Ev::lift();
	});

	auto act3 = std::move(act1) + std::move(act2);

	auto act4 = std::move(act3) + (Ev::lift().then([&]() {
		assert(flag1);
		assert(flag2);
		assert(!flag3);
		flag3 = true;
		return Ev::lift();
	}));

	act4 += Ev::lift().then([&]() {
		assert(flag1);
		assert(flag2);
		assert(flag3);
		assert(!flag4);
		flag4 = true;
		return Ev::lift();
	});

	auto res = Ev::start(std::move(act4).then([&]() {
		assert(flag1);
		assert(flag2);
		assert(flag3);
		assert(flag4);
		return Ev::lift(0);
	}));

	assert(flag1);
	assert(flag2);
	assert(flag3);
	assert(flag4);
	assert(res == 0);
	return res;
}
