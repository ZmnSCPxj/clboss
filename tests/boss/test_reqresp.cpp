#undef NDEBUG
#include"Boss/ModG/ReqResp.hpp"
#include"Ev/Io.hpp"
#include"Ev/map.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<assert.h>
#include<vector>

namespace {

/* Example req-resp module that just does a trivial processing.  */
struct InNumber {
	void* requester;
	std::uint32_t number;
};
struct OutNumber {
	void* requester;
	std::uint32_t number;
};
class Incrementer {
private:
	S::Bus& bus;

public:
	Incrementer() =delete;
	Incrementer(Incrementer&&) =delete;
	Incrementer(Incrementer const&) =delete;

	explicit
	Incrementer( S::Bus& bus_
		   ) : bus(bus_) {
		bus.subscribe<InNumber>([this](InNumber const& in) {
			return bus.raise(OutNumber{ in.requester
						  , in.number + 1
						  });
		});
	}
};

}

int main() {
	auto bus = S::Bus();
	auto incr = Util::make_unique<Incrementer>(bus);
	auto rr = Boss::ModG::ReqResp<InNumber, OutNumber>
		(bus);

	auto code = Ev::lift().then([&]() {

		return rr.execute(InNumber{nullptr, 42});
	}).then([&](OutNumber n) {
		assert(n.number == 43);

		/* Parallel execution.  */
		auto v = std::vector<std::uint32_t>{5, 6, 7, 8};
		auto f = [&](std::uint32_t v) {
			return Ev::yield().then([&rr, v]() {
				return rr.execute(InNumber{nullptr, v});
			}).then([](OutNumber n) {
				return Ev::lift(n.number);
			});
		};
		return Ev::map(std::move(f), std::move(v));
	}).then([&](std::vector<std::uint32_t> nums) {
		assert((nums == std::vector<std::uint32_t>{6, 7, 8, 9}));

		return Ev::lift(0);
	});

	return Ev::start(code);
}
