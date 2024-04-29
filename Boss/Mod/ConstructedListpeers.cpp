#include"Boss/Mod/ConstructedListpeers.hpp"
#include"Jsmn/Object.hpp"
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

std::ostream& operator<<(std::ostream& os, Boss::Mod::ConstructedListpeers const& o) {
	for (auto p : o) {
		os << p.first << ':'
                   << "connected: " << p.second.connected
                   << ", channels: ";
                for (auto c : p.second.channels) {
                  os << c;
                }
        }
	return os;
}


}}
