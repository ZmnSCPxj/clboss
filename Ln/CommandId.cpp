#include"Jsmn/Object.hpp"
#include"Ln/CommandId.hpp"
#include"Util/make_unique.hpp"
#include<sstream>

namespace {

std::uint64_t string_to_u64(std::string const& s) {
	auto is = std::istringstream(s);
	auto num = std::uint64_t();
	is >> num;
	return num;
}

}

namespace Ln {

std::unique_ptr<CommandId>
command_id_from_jsmn_object(Jsmn::Object const& j) {
	if (j.is_number()) {
		auto s_id = j.direct_text();
		return Util::make_unique<CommandId>(
			CommandId::left(string_to_u64(s_id))
		);
	} else if (j.is_string()) {
		return Util::make_unique<CommandId>(
			CommandId::right(std::string(j))
		);
	}
	return nullptr;
}

}
