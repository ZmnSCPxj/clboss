#undef NDEBUG
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include<assert.h>
#include<sstream>

int main() {
	{
		auto s = Json::Out()
			.start_object()
				.field("number", 42)
			.end_object()
			.output()
			;

		auto is = std::istringstream(s);
		auto jsmn = Jsmn::Object();
		is >> jsmn;

		assert(jsmn.is_object());
		assert(jsmn.size() == 1);
		assert(jsmn.has("number"));
		assert(jsmn["number"].is_number());
		assert(((double)jsmn["number"]) == 42);
		assert(!jsmn.has("nonexistent"));
	}

	{
		auto s = Json::Out()
			.start_array()
				.start_object()
				.end_object()
				.entry(true)
			.end_array()
			.output()
			;

		auto is = std::istringstream(s);
		auto jsmn = Jsmn::Object();
		is >> jsmn;

		assert(jsmn.is_array());
		assert(jsmn.size() == 2);
		assert(jsmn[0].is_object());
		assert(jsmn[0].size() == 0);
		assert(jsmn[1].is_boolean());
		assert((bool)jsmn[1] == true);
	}

	{
		auto js = Json::Out();
		auto arr = js.start_array();
		for (auto i = 0; i < 8; ++i) {
			arr.entry(i);
		}
		arr.end_array();
		auto s = js.output();

		auto is = std::istringstream(s);
		auto jsmn = Jsmn::Object();
		is >> jsmn;

		assert(jsmn.is_array());
		assert(jsmn.size() == 8);
		for (auto i = 0; i < 8; ++i) {
			assert(jsmn[i].is_number());
			assert(((double)jsmn[i]) == i);
		}
	}

	return 0;
}
