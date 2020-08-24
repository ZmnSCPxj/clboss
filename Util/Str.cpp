#include<algorithm>
#include<cctype>
#include<iomanip>
#include<sstream>
#include"Util/Str.hpp"

namespace Util {
namespace Str {

std::string hexbyte(std::uint8_t v) {
	std::ostringstream os;
	os << std::hex << std::setfill('0') << std::setw(2);
	/* uint8_t might be a char, which would confuse iostreams.
	 * Individual char is printed out as an ASCII character.
	 * Cast to an unsigned int to ensure it is printed as a
	 * number.
	 */
	os << ((unsigned int) v);
	return os.str();
}

std::string hexdump(void const* vp, std::size_t s) {
	auto os = std::ostringstream();
	auto p = (std::uint8_t const*) vp;
	for (auto i = std::size_t(0); i < s; ++p, ++i)
		os << hexbyte(*p);
	return os.str();
}

namespace {

std::uint8_t parse_hex(char c) {
	if (('0' <= c) && (c <= '9')) {
		return (std::uint8_t) (c & 0xF);
	} else if ((('a' <= c) && (c <= 'f')) ||
		   (('A' <= c) && (c <= 'F'))) {
		return (std::uint8_t) ((c + 9) & 0xF);
	} else {
		char s[2];
		s[0] = c;
		s[1] = 0;
		throw HexParseFailure("Non-hex character: " + std::string(s));
	}
}

std::uint8_t parse_hex_byte(char c0, char c1) {
	return (parse_hex(c0) << 4) | (parse_hex(c1));
}

}

std::vector<std::uint8_t> hexread(std::string const& s) {
	if ((s.length() % 2) != 0)
		throw HexParseFailure("String length must be even.");

	auto buflen = s.length() / 2;
	auto buf = std::vector<std::uint8_t>(buflen);

	for (auto i = std::size_t(0); i < buflen; ++i) {
		buf[i] = parse_hex_byte(s[i * 2], s[i * 2 + 1]);
	}

	return buf;
}

bool ishex(std::string const& s) {
	if ((s.size() % 2) != 0)
		return false;
	for (auto const& c : s) {
		if ( ('0' <= c && c <= '9')
		  || ('a' <= c && c <= 'f')
		  || ('A' <= c && c <= 'F')
		   )
			continue;
		return false;
	}
	return true;
}

std::string trim(std::string const& s) {
	auto start = std::find_if_not(s.begin(), s.end(), isspace);
	/* If all spaces, empty string.  */
	if (start == s.end())
		return "";

	auto rend = std::find_if_not(s.rbegin(), s.rend(), isspace);
	auto end = rend.base();

	return std::string(start, end);
}

}
}
