#ifndef BOSS_MOD_ONCHAINFUNDSANNOUNCER_HPP
#define BOSS_MOD_ONCHAINFUNDSANNOUNCER_HPP

#include"Boss/ModG/ReqResp.hpp"
#include<string>

namespace Boss { namespace Mod { class Rpc; }}
namespace Boss { namespace Msg { struct RequestGetOnchainIgnoreFlag; }}
namespace Boss { namespace Msg { struct ResponseGetOnchainIgnoreFlag; }}
namespace Ev { template<typename a> class Io; }
namespace Jsmn { class Object; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

class OnchainFundsAnnouncer {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;
	ModG::ReqResp< Msg::RequestGetOnchainIgnoreFlag
		     , Msg::ResponseGetOnchainIgnoreFlag
		     > get_ignore_rr;

	void start();
	Ev::Io<void> on_block();
	Ev::Io<void> announce();
	Ev::Io<void> fail(std::string const&, Jsmn::Object res);
	Ev::Io<Jsmn::Object> fundpsbt();

public:
	OnchainFundsAnnouncer(OnchainFundsAnnouncer const&) =delete;
	OnchainFundsAnnouncer(OnchainFundsAnnouncer&&) =delete;

	~OnchainFundsAnnouncer();
	OnchainFundsAnnouncer(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_ONCHAINFUNDSANNOUNCER_HPP) */
