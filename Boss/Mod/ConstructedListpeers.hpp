#ifndef BOSS_MOD_CONSTRUCTEDLISTPEERS_HPP
#define BOSS_MOD_CONSTRUCTEDLISTPEERS_HPP

#include<map>
#include"Ln/NodeId.hpp"
#include"Jsmn/Object.hpp"

namespace Boss { namespace Mod {

/*
 * IMPORTANT - the msg is no longer directly obtained from
 * `listpeers` but rather is constructed by "convolving" the value
 * from `listpeerchannels`.  Specifically, the top level `peer`
 * objects are non-standard and only have what CLBOSS uses ...
 */

/** class Boss::Mod::ConstructedListpeers
 */
struct ConstructedListpeer {
	bool connected;
	std::vector<Jsmn::Object> channels;
};

typedef std::map<Ln::NodeId, ConstructedListpeer> ConstructedListpeers;

std::ostream& operator<<(std::ostream& os, ConstructedListpeers const& o);


/* for unit tests w/ legacy listpeer setups */
Boss::Mod::ConstructedListpeers convert_legacy_listpeers(Jsmn::Object const & legacy_listpeers);

}}
#endif /* !defined(BOSS_MOD_CONSTRUCTEDLISTPEERS_HPP) */
