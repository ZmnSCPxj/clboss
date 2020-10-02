#ifndef BOSS_MOD_CHANNELFINDERBYDISTANCE_HPP
#define BOSS_MOD_CHANNELFINDERBYDISTANCE_HPP

#include"Ln/NodeId.hpp"

namespace Boss { namespace Mod { class Rpc; }}
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ChannelFinderByDistance
 *
 * @brief Finds channel candidates according to
 * distance from our node: the more distant, the
 * more desirable we consider them.
 */
class ChannelFinderByDistance {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;
	Ln::NodeId self_id;

	class Run;

	void start();

public:
	ChannelFinderByDistance() =delete;
	ChannelFinderByDistance(ChannelFinderByDistance&&) =delete;
	ChannelFinderByDistance(ChannelFinderByDistance const&) =delete;

	explicit
	ChannelFinderByDistance( S::Bus& bus_
			       ) : bus(bus_)
				 , rpc(nullptr)
				 { start(); }
};

}}

#endif /* !defined(BOSS_MOD_CHANNELFINDERBYDISTANCE_HPP) */
