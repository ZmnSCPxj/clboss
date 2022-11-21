#undef NDEBUG
#include"Ln/CommandId.hpp"
#include"Jsmn/Object.hpp"
#include<assert.h>
#include<sstream>

namespace {

std::unique_ptr<Ln::CommandId>
from_json(std::string jtxt) {
	auto jobj = Jsmn::Object();
	auto is = std::istringstream(jtxt + "\n");
	is >> jobj;
	return Ln::command_id_from_jsmn_object(jobj);
}

}

int main() {
	using Ln::CommandId;

	{
		auto cid = from_json("{}");
		assert(!cid);

		cid = from_json("42");
		assert(cid);
		assert(*cid == CommandId::left(42));

		cid = from_json("\"a string key\"");
		assert(cid);
		assert(*cid == CommandId::right("a string key"));
	}

	return 0;
}

