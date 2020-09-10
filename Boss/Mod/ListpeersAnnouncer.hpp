#ifndef BOSS_MOD_LISTPEERSANNOUNCER_HPP
#define BOSS_MOD_LISTPEERSANNOUNCER_HPP

namespace Boss { namespace Mod { class Rpc; }}
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ListpeersAnnouncer
 *
 * @brief announces `listpeers` at `init` and
 * every 10 minutes thereafter.
 */
class ListpeersAnnouncer {
private:
	S::Bus& bus;
	Boss::Mod::Rpc *rpc;

	void start();

public:
	ListpeersAnnouncer() =delete;
	ListpeersAnnouncer(ListpeersAnnouncer const&) =delete;
	ListpeersAnnouncer( S::Bus& bus_
			  ) : bus(bus_)
			    , rpc(nullptr)
			    { start(); }
};

}}

#endif /* BOSS_MOD_LISTPEERSANNOUNCER_HPP */
