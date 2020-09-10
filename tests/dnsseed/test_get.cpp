#undef NDEBUG
#include"DnsSeed/get.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include<assert.h>

int main() {
	auto code = Ev::lift().then([]() {
		return DnsSeed::can_get();
	}).then([&](std::string err) {
		if (err != "")
			/* Trivial exit.  */
			return Ev::lift(0);
		return Ev::lift().then([]() {
			return DnsSeed::get("lseed.bitcoinstats.com");
		}).then([](std::vector<std::string> results) {
			assert(results.size() > 0);

			return Ev::lift(0);
		});
	});
	return Ev::start(code);
}
