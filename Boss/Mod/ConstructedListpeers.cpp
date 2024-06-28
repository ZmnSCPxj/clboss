#include"Boss/Mod/ConstructedListpeers.hpp"
#include"Jsmn/Object.hpp"
#include"Ln/NodeId.hpp"
#include"Util/stream_elements.hpp"
#include<sstream>

namespace Boss { namespace Mod {

/*
 * IMPORTANT - the msg is no longer directly obtained from
 * `listpeers` but rather is constructed by "convolving" the value
 * from `listpeerchannels`.  Specifically, the top level `peer`
 * objects are non-standard and only have what CLBOSS uses ...
 */

/* This helper is used by tests which use the old listpeers format */
Boss::Mod::ConstructedListpeers convert_legacy_listpeers(Jsmn::Object const & legacy_listpeers) {
	Boss::Mod::ConstructedListpeers cpeers;
	for (auto p : legacy_listpeers) {
		if (!p.has("id"))
			continue;
		auto id = Ln::NodeId(std::string(p["id"]));
		cpeers[id].connected = false;
		if (p.has("connected")) {
			auto connected = p["connected"];
			cpeers[id].connected = connected.is_boolean() && bool(connected);
		}
		auto cs = p["channels"];
		for (auto c : cs) {
			cpeers[id].channels.push_back(c);
		}
	}
	return cpeers;
}

std::ostream& operator<<(std::ostream& os, ConstructedListpeer const& o) {
	os << "(ConstructedListpeer): {"
	   << "connected: " << o.connected
	   << ", channels: ";
	Util::stream_elements(os, o.channels);
	os << "}";
	return os;
}

std::ostream& operator<<(std::ostream& os, ConstructedListpeers const& o) {
	os << "(ConstructedListpeers): {";
	Util::stream_elements(os, o);
	os << "}";
	return os;
}

std::ostream& operator<<(std::ostream& os, const std::pair<Ln::NodeId, ConstructedListpeer>& p) {
    os << "{" << p.first << ": " << p.second << "}";
    return os;
}

}}
