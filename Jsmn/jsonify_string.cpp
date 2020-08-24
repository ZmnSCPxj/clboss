#include"Jsmn/Detail/Str.hpp"
#include"Jsmn/jsonify_string.hpp"

namespace Jsmn {

std::string jsonify_string(std::string const& s) {
	return "\"" + Detail::Str::to_escaped(s) + "\"";
}

}
