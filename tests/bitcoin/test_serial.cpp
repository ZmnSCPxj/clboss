#undef NDEBUG
#include"Bitcoin/le.hpp"
#include"Bitcoin/varint.hpp"
#include"Ln/Amount.hpp"
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

void test_varint(char const* hexstr, std::uint64_t expected) {
	auto is = to_bytestream(hexstr);
	auto v = std::uint64_t();
	is >> Bitcoin::varint(v);
	assert(v == expected);

	auto os = std::ostringstream();
	os << Bitcoin::varint(v);
	assert(from_bytestream(os) == hexstr);
}
template<typename a>
void test_le(char const* hexstr, a expected) {
	auto is = to_bytestream(hexstr);
	auto v = a();
	is >> Bitcoin::le(v);
	assert(v == expected);

	auto os = std::ostringstream();
	os << Bitcoin::le(v);
	assert(from_bytestream(os) == hexstr);
}

}

int main() {
	test_varint("00", 0);
	test_varint("20", 32);
	test_varint("2a", 42);
	test_varint("f0", 0xF0);
	test_varint("fd1234", 0x3412);
	test_varint("fd2009", 0x0920);
	test_varint("fe87654321", 0x21436587);
	test_varint("fe09654320", 0x20436509);
	test_varint("ff0123456789abcdef", 0xEFCDAB8967452301);

	test_le<std::uint16_t>("ff20", 0x20ff);
	test_le<std::int16_t>("feff", -2);
	test_le<std::uint32_t>("20abcdef", 0xefcdab20);
	test_le<std::int32_t>("00000080", -2147483648);
	test_le<std::uint64_t>("fedcba9876543210", 0x1032547698badcfe);
	test_le<std::int64_t>("e0ffffffffffffff", -32);
	test_le<Ln::Amount>("2000000000000000", Ln::Amount::sat(32));

	return 0;
}

