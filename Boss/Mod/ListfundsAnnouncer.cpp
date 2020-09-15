#include"Boss/Mod/ListfundsAnnouncer.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ListfundsResult.hpp"
#include"Boss/Msg/Timer10Minutes.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include<sstream>

namespace Boss { namespace Mod {

void ListfundsAnnouncer::start() {
	bus.subscribe< Msg::Init
		     >([this](Msg::Init const& init) {
		rpc = &init.rpc;

		/* Also trigger at init.  */
		return Boss::concurrent(on_10_minutes());
	});
	bus.subscribe< Msg::Timer10Minutes
		     >([this](Msg::Timer10Minutes const& _) {
		if (!rpc)
			return Ev::lift();

		return Boss::concurrent(on_10_minutes());
	});
}
Ev::Io<void> ListfundsAnnouncer::on_10_minutes() {
	return Ev::lift().then([this]() {
		return rpc->command("listfunds", Json::Out::empty_object());
	}).then([this](Jsmn::Object res) {
		if (!res.is_object())
			return fail("listfunds did not return object", res);
		if (!res.has("channels"))
			return fail("listfunds did not return channels", res);
		if (!res.has("outputs"))
			return fail("listfunds did not return outputs", res);
		auto channels = res["channels"];
		auto outputs = res["outputs"];
		if (!channels.is_array())
			return fail("listfunds channels not array", channels);
		if (!outputs.is_array())
			return fail("listfunds outputs not array", outputs);

		return bus.raise(Msg::ListfundsResult{
			std::move(outputs), std::move(channels)
		});
	});
}
Ev::Io<void>
ListfundsAnnouncer::fail(std::string const& msg, Jsmn::Object res) {
	auto os = std::ostringstream();
	os << res;
	return Boss::log( bus, Error
			, "ListfundsAnnouncer: %s: %s"
			, msg.c_str()
			, os.str().c_str()
			);
}

}}
