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
 *
 * IMPORTANT - this msg is no longer directly obtained from
 * `listpeers` but rather is constructed by "convolving" the value
 * from `listpeerchannels`.  Specifically, the top level `peer`
 * objects are non-standard and only have what CLBOSS uses ...
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
