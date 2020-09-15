#ifndef BOSS_MOD_ONCHAINFUNDSANNOUNCER_HPP
#define BOSS_MOD_ONCHAINFUNDSANNOUNCER_HPP

#include<string>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
namespace Jsmn { class Object; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

class OnchainFundsAnnouncer {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;

	void start();
	Ev::Io<void> on_block();
	Ev::Io<void> fail(std::string const&, Jsmn::Object res);

public:
	OnchainFundsAnnouncer(OnchainFundsAnnouncer const&) =delete;
	OnchainFundsAnnouncer(S::Bus& bus_) : bus(bus_), rpc(nullptr) {
		start();
	}
};

}}

#endif /* !defined(BOSS_MOD_ONCHAINFUNDSANNOUNCER_HPP) */
