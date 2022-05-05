#undef NDEBUG
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Shutdown.hpp"
#include"Jsmn/Parser.hpp"
#include"Json/Out.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"Ev/start.hpp"
#include"S/Bus.hpp"
#include<assert.h>

int main() {
	auto bus = S::Bus();
	Boss::Mod::Waiter waiter (bus);
	Ev::ThreadPool tp;

	/* Generate a large JSON text.  */
	auto js = Json::Out();
	auto arr = js.start_array();
	for (auto i = 0; i < 200000; ++i) {
		arr
			.start_object()
				.field("dummy", "not a tsundere")
				.start_object("subobj")
					.field("i", i)
				.end_object()
			.end_object()
			;
	}
	arr.end_array();
	auto sample_text = js.output();

	auto code = Ev::lift().then([&]() {
		auto act = tp.background<std::vector<Jsmn::Object>>([&]() {
			/* Parse our generated JSON.  */
			Jsmn::Parser parser;
			return parser.feed(sample_text);
		});
		return waiter.timed(5.0, std::move(act));
	}).then([&](std::vector<Jsmn::Object> result) {
		assert(result.size() == 1);
		assert(result[0].is_array());
		return Ev::lift();
	}).catching<Boss::Mod::Waiter::TimedOut>([](Boss::Mod::Waiter::TimedOut const&) {
		assert(0); /* Should not happen.  */
		return Ev::lift();
	}).then([&]() {
		return bus.raise(Boss::Shutdown{});
	}).then([]() {
		return Ev::lift(0);
	});

	return Ev::start(code);
}
