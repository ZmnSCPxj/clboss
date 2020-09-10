#include"Ln/NodeId.hpp"
#include"Util/Str.hpp"
#include<algorithm>
#include<stdexcept>
#include<string.h>

namespace {

bool all_zeros(std::string const& s) {
	return std::all_of( s.begin(), s.end()
			  , [](char c) { return c == '0'; }
			  );
}

}

namespace Ln {

NodeId::NodeId(std::string const& s) {
	/* Parse.  */
	if (!valid_string(s))
		throw std::range_error(
			std::string("Ln::NodeId: not node ID: ") + s
		);

	if (all_zeros(s)) {
		pimpl = nullptr;
		return;
	}

	auto val = Util::Str::hexread(s);

	/* Create a temporary writeable Impl and copy the data over.  */
	auto tmp = std::make_shared<Impl>();
	std::copy(val.begin(), val.end(), tmp->raw);
	/* Now promote it to our Impl const*.  */
	pimpl = std::move(tmp);
}

bool NodeId::valid_string(std::string const& s) {
	if (s.size() != 66)
		return false;
	if (!Util::Str::ishex(s))
		return false;
	if (s[0] != '0')
		return false;
	if ( s[1] != '2' && s[1] != '3' && !all_zeros(s))
		return false;
	return true;
}

NodeId::operator std::string() const {
	if (!pimpl)
		return "000000000000000000000000000000000000000000000000000000000000000000";
	return Util::Str::hexdump(pimpl->raw, sizeof(pimpl->raw));
}

bool NodeId::equality_check(NodeId const& o) const {
	if (!o.pimpl && pimpl)
		return false;
	if (o.pimpl && !pimpl)
		return false;
	/* Both being nullptr is impossible at this point, as
	 * the operator== would have returned then and there.  */

	/* Non-constant-time compare!
	 * This should be fine since node IDs are public keys and
	 * not secrets.
	 */
	return 0 == memcmp(pimpl->raw, o.pimpl->raw, sizeof(pimpl->raw));
}

std::istream& operator>>(std::istream& is, NodeId& n) {
	auto tmp = std::string();
	is >> tmp;
	n = NodeId(tmp);
	return is;
}

std::ostream& operator<<(std::ostream& os, NodeId const& n) {
	return os << (std::string) n;
}

}
