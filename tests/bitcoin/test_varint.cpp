#undef NDEBUG
#include"Bitcoin/varint.hpp"
#include"Util/Str.hpp"
#include<algorithm>
#include<assert.h>
#include<sstream>

namespace {

std::istringstream to_bytestream(std::string const& hexstr) {
	auto buff = Util::Str::hexread(hexstr);
	auto s = std::string();
	s.resize(buff.size());
	std::copy( buff.begin(), buff.end(), s.begin());
	return std::istringstream(s);
}
std::string from_bytestream(std::ostringstream& os) {
	auto s = os.str();
	return Util::Str::hexdump(&s[0], s.size());
}

void dotest(char const* hexstr, std::uint64_t expected) {
	auto is = to_bytestream(hexstr);
	auto v = std::uint64_t();
	is >> Bitcoin::varint(v);
	assert(v == expected);

	auto os = std::ostringstream();
	os << Bitcoin::varint(v);
	assert(from_bytestream(os) == hexstr);
}

}

int main() {
	dotest("00", 0);
	dotest("2a", 42);
	dotest("f0", 0xF0);
	dotest("fd1234", 0x3412);
	dotest("fe87654321", 0x21436587);
	dotest("ff0123456789abcdef", 0xEFCDAB8967452301);

	return 0;
}

