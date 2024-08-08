#include"Ln/Scid.hpp"
#include"Util/BacktraceException.hpp"
#include<algorithm>
#include<sstream>
#include<stdexcept>

namespace {

bool is_digit(char c) {
	return '0' <= c && c <= '9';
}

template<typename It>
bool parse_number(It b, It e, std::uint64_t& val, std::uint64_t mask) {
	val = 0;
	/* Does not filter out '000001'.  */
	while (b != e) {
		auto c = *b;
		if (!is_digit(c))
			return false;
		auto new_val = val * 10 + std::uint64_t(c - '0');
		new_val = new_val & mask;
		/* Overflow?  */
		if (new_val < val)
			return false;

		val = new_val;
		++b;
	}
	return true;
}

bool validate_and_parse( std::string const& s
		       , std::uint64_t& b
		       , std::uint64_t& t
		       , std::uint64_t& o
		       ) {
	auto it = std::find(s.begin(), s.end(), 'x');
	if (it == s.end())
		return false;
	auto it2 = std::find(it + 1, s.end(), 'x');
	if (it2 == s.end())
		return false;

	/* Not empty.  */
	if (it - s.begin() == 0)
		return false;
	if (it2 - (it + 1) == 0)
		return false;
	if (s.end() - (it2 + 1) == 0)
		return false;

	if (!parse_number(s.begin(), it, b, 0xFFFFFF))
		return false;
	if (!parse_number(it + 1, it2, t, 0xFFFFFF))
		return false;
	if (!parse_number(it2 + 1, s.end(), o, 0xFFFF))
		return false;

	return true;
}

}

namespace Ln {

Scid::operator std::string() const {
	auto os = std::ostringstream();
	auto b = (val >> 40) & 0xFFFFFF;
	auto t = (val >> 16) & 0xFFFFFF;
	auto o = (val >> 0) & 0xFFFF;
	os << std::dec << b << "x" << t << "x" << o;
	return os.str();
}
bool Scid::valid_string(std::string const& s) {
	auto b_ = std::uint64_t();
	auto t_ = std::uint64_t();
	auto o_ = std::uint64_t();
	return validate_and_parse(s, b_, t_, o_);
}
Scid::Scid(std::string const& s) {
	auto b = std::uint64_t();
	auto t = std::uint64_t();
	auto o = std::uint64_t();
	if (!validate_and_parse(s, b, t, o))
		throw Util::BacktraceException<std::invalid_argument>(std::string("Not an SCID: ") + s);
	val = (b << 40)
	    | (t << 16)
	    | (o << 0)
	    ;
}

}
