#ifndef BOSS_MOD_LISTFUNDSANNOUNCER_HPP
#define BOSS_MOD_LISTFUNDSANNOUNCER_HPP

#include<string>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
namespace Jsmn { class Object; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ListfundsAnnouncer
 *
 * @brief Periodically performs `listfunds` and
 * announces it.
 */
class ListfundsAnnouncer {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;

	void start();
	Ev::Io<void> on_10_minutes();
	Ev::Io<void> fail(std::string const&, Jsmn::Object);

public:
	ListfundsAnnouncer() =delete;
	ListfundsAnnouncer(ListfundsAnnouncer const&) =delete;
	ListfundsAnnouncer(ListfundsAnnouncer&&) =delete;

	ListfundsAnnouncer(S::Bus& bus_
			  ) : bus(bus_), rpc(nullptr)
			    { start(); }
};

}}

#endif /* !defined(BOSS_MOD_LISTFUNDSANNOUNCER_HPP) */
