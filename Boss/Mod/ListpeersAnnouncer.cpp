#include"Boss/Mod/ListpeersAnnouncer.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ListpeersResult.hpp"
#include"Boss/Msg/Timer10Minutes.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include<assert.h>
#include<sstream>

namespace Boss { namespace Mod {

void ListpeersAnnouncer::start() {
	auto invalid_listpeers = [this](Jsmn::Object result) {
		auto os = std::ostringstream();
		os << result;
		return Boss::log( bus, Error
				, "ListpeersAnnouncer: invalid result from "
				  "`listpeers`."
				);
	};
	auto do_listpeers = [ this
			    , invalid_listpeers
			    ](bool initial) {
		assert(rpc);
		return rpc->command("listpeers"
				   , Json::Out::empty_object()
				   ).then([ this
					  , initial
					  , invalid_listpeers
					  ](Jsmn::Object result) {
			if (!result.is_object() || !result.has("peers"))
				return invalid_listpeers(std::move(result));
			auto peers = result["peers"];
			if (!peers.is_array())
				return invalid_listpeers(std::move(result));

			return bus.raise(Msg::ListpeersResult{
				std::move(peers), initial
			});
		});
	};
	bus.subscribe<Msg::Init
		     >([this, do_listpeers](Msg::Init const& init) {
		rpc = &init.rpc;
		return do_listpeers(true);
	});
	bus.subscribe<Msg::Timer10Minutes
		     >([this, do_listpeers](Msg::Timer10Minutes const& _) {
		return do_listpeers(false);
	});
}

}}
