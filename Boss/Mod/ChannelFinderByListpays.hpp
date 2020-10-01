#ifndef BOSS_MOD_CHANNELFINDERBYLISTPAYS_HPP
#define BOSS_MOD_CHANNELFINDERBYLISTPAYS_HPP

#include"Ln/NodeId.hpp"
#include<set>

namespace Boss { namespace Mod { class Rpc; }}
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ChannelFinderByListpays
 *
 * @brief Proposes the nodes that you have paid, or
 * tried to pay, as channel candidates.
 */
class ChannelFinderByListpays {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;

	/* Peers we already have channels with.  */
	std::set<Ln::NodeId> channels;

	void start();

public:
	ChannelFinderByListpays() =delete;
	ChannelFinderByListpays(ChannelFinderByListpays&&) =delete;
	ChannelFinderByListpays(ChannelFinderByListpays const&) =delete;

	explicit
	ChannelFinderByListpays( S::Bus& bus_
			       ) : bus(bus_)
				 , rpc(nullptr)
				 { start(); }
};

}}

#endif /* !defined(BOSS_MOD_CHANNELFINDERBYLISTPAYS_HPP) */
