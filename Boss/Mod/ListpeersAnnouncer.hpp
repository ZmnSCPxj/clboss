#ifndef BOSS_MOD_LISTPEERSANNOUNCER_HPP
#define BOSS_MOD_LISTPEERSANNOUNCER_HPP

#include<memory>

namespace Boss { namespace ModG { class RpcProxy; }}
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
	std::unique_ptr<Boss::ModG::RpcProxy> rpc;

	void start();

public:
	ListpeersAnnouncer() =delete;
	ListpeersAnnouncer(ListpeersAnnouncer const&) =delete;
	~ListpeersAnnouncer();
	ListpeersAnnouncer(S::Bus& bus);
};

}}

#endif /* BOSS_MOD_LISTPEERSANNOUNCER_HPP */
