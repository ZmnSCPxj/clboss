#ifndef BOSS_MOD_LISTPAYSHANDLER_HPP
#define BOSS_MOD_LISTPAYSHANDLER_HPP

#include"Sha256/Hash.hpp"
#include<vector>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ListpaysHandler
 *
 * @brief Handles `Boss::Msg::RequestListpays`
 * messages and emits `Boss::Msg::ResponseListpays`
 * in response.
 */
class ListpaysHandler {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;
	std::vector<Sha256::Hash> pending;

	void start();
	Ev::Io<void> listpays(Sha256::Hash);

public:
	ListpaysHandler() =delete;
	ListpaysHandler(ListpaysHandler&&) =delete;
	ListpaysHandler(ListpaysHandler const&) =delete;

	explicit
	ListpaysHandler(S::Bus& bus_) : bus(bus_), rpc(nullptr) {
		start();
	}
};

}}

#endif /* !defined(BOSS_MOD_LISTPAYSHANDLER_HPP) */
