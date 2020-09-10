#undef NDEBUG
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"
#include<assert.h>

int main() {
	auto code = Ev::yield().then([]() {
		throw int(42);
		return Ev::lift(1);
	}).catching<int>([](int _) {
		return Ev::lift(0);
	});
	return Ev::start(code);
}
