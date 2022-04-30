#include"Boss/Mod/ListpeersAnnouncer.hpp"
#include"Boss/ModG/RpcProxy.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ListpeersResult.hpp"
#include"Boss/Msg/Timer10Minutes.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
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
		     >([do_listpeers](Msg::Init const& init) {
		return Boss::concurrent(do_listpeers(true));
	});
	bus.subscribe<Msg::Timer10Minutes
		     >([do_listpeers](Msg::Timer10Minutes const& _) {
		return Boss::concurrent(do_listpeers(false));
	});
}

ListpeersAnnouncer::ListpeersAnnouncer(S::Bus& bus_)
	: bus(bus_)
	, rpc(Util::make_unique<ModG::RpcProxy>(bus))
	{ start(); }

ListpeersAnnouncer::~ListpeersAnnouncer() =default;

}}
