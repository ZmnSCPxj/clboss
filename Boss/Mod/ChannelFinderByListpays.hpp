#ifndef BOSS_MOD_CHANNELFINDERBYLISTPAYS_HPP
#define BOSS_MOD_CHANNELFINDERBYLISTPAYS_HPP

#include"Jsmn/Object.hpp"
#include"Ln/NodeId.hpp"
#include<cstddef>
#include<map>
#include<set>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
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

	bool running;
	/* Used in a run.  */
	std::map<Ln::NodeId, std::size_t> payees;
	Jsmn::Object pays;
	Jsmn::Object::iterator it;
	std::size_t count;

	Ev::Io<void> extract_payees_loop();

	void start();

public:
	ChannelFinderByListpays() =delete;
	ChannelFinderByListpays(ChannelFinderByListpays&&) =delete;
	ChannelFinderByListpays(ChannelFinderByListpays const&) =delete;

	explicit
	ChannelFinderByListpays( S::Bus& bus_
			       ) : bus(bus_)
				 , rpc(nullptr)
				 , running(false)
				 { start(); }
};

}}

#endif /* !defined(BOSS_MOD_CHANNELFINDERBYLISTPAYS_HPP) */
