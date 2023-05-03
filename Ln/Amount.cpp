#include"Ln/Amount.hpp"
#include<algorithm>
#include<sstream>
#include<stdexcept>

namespace Ln {

/* C-Lightning supports a variety of amount formats, but consistently
 * outputs in millisatoshi with explicit `msat` suffixes.
 */
bool Amount::valid_string(std::string const& s) {
	if (s.size() < 5)
		return false;
	if (std::string(s.end() - 4, s.end()) != "msat")
		return false;
	bool flag = std::all_of( s.begin(), s.end() - 4
			       , [](char c) { return '0' <= c && c <= '9'; }
			       );
	if (!flag)
		return false;
	/*   21,000,000 BTC * 100,000,000 sat/BTC * 1,000 msat/sat
	 * = 2,100,000,000,000,000,000
	 * is 13 digits.
	 * The "4" is "msat".
	 */
	if (s.size() > 13 + 4)
		return false;
	return true;
}

bool Amount::valid_object(Jsmn::Object const& o) {
	if (o.is_number())
		return true;
	else if (o.is_string())
		return valid_string(std::string(o));
	else
		return false;
}

Amount
Amount::object(Jsmn::Object const& o) {
	if (o.is_number())
		return Amount::msat(std::uint64_t(double(o)));
	else if (o.is_string())
		return Amount(std::string(o));
	else
		throw std::invalid_argument("Ln::Amount json object invalid.");
}

Amount::Amount(std::string const& s) {
	if (!valid_string(s))
		throw std::invalid_argument("Ln::Amount string invalid.");
	auto is = std::istringstream(std::string(s.begin(), s.end() - 4));
	is >> v;
}
Amount::operator std::string() const {
	auto os = std::ostringstream();
	os << v << "msat";
	return os.str();
}

}
