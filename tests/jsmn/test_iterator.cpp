#undef NDEBUG
#include"Jsmn/Object.hpp"
#include"Jsmn/Parser.hpp"
#include"Util/stringify.hpp"
#include<assert.h>
#include<vector>

int main() {
	auto vectors = std::vector<std::string>{
		R"JSON(
		[ 0
		, 1
		, 2
		, 3
		]
		)JSON",
		R"JSON(
		[ {"asdf": 42}
		, {"nested" : {"nested": {"leaf": 0}}}
		, [0, 1, 2, {}]
		, 3
		]
		)JSON",
		R"JSON(
		[]
		)JSON",
	};
	for (auto const& v: vectors) {
		Jsmn::Parser p;
		auto jv = p.feed(v);
		auto js = jv[0];

		auto i = std::size_t(0);
		for (auto entry: js) {
			auto a = Util::stringify(entry);
			auto b = Util::stringify(js[i]);
			assert(a == b);
			++i;
		}
		assert(i == js.size());
	}

	return 0;
}
