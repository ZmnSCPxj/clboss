#include"Boss/Mod/ListpeersAnnouncer.hpp"
#include"Boss/Mod/ConstructedListpeers.hpp"
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

/*
 * IMPORTANT - the msg is no longer directly obtained from
 * `listpeers` but rather is constructed by "convolving" the value
 * from `listpeerchannels`.  Specifically, the top level `peer`
 * objects are non-standard and only have what CLBOSS uses ...
 */

void ListpeersAnnouncer::start() {
	auto invalid_listpeerchannels = [this](Jsmn::Object result) {
		auto os = std::ostringstream();
		os << result;
		return Boss::log( bus, Error
				, "ListpeersAnnouncer: invalid result from "
				  "`listpeerchannels`."
				);
	};
	auto do_listpeerchannels = [ this
			    , invalid_listpeerchannels
			    ](bool initial) {
		return rpc->command("listpeerchannels"
				   , Json::Out::empty_object()
				   ).then([ this
					  , initial
					  , invalid_listpeerchannels
					  ](Jsmn::Object result) {
			if (!result.is_object() || !result.has("channels"))
				return invalid_listpeerchannels(std::move(result));
			auto channels = result["channels"];
			if (!channels.is_array())
				return invalid_listpeerchannels(std::move(result));

			Boss::Mod::ConstructedListpeers cpeers;
			for (auto c : channels) {
				auto id = Ln::NodeId(std::string(c["peer_id"]));
				auto connected = c["peer_connected"];
				cpeers[id].connected = connected.is_boolean() && bool(connected);
				cpeers[id].channels.push_back(c);
			}

			return bus.raise(Msg::ListpeersResult{
				std::move(cpeers), initial
			});
		});
	};
	bus.subscribe<Msg::Init
		     >([do_listpeerchannels](Msg::Init const& init) {
		return Boss::concurrent(do_listpeerchannels(true));
	});
	bus.subscribe<Msg::Timer10Minutes
		     >([do_listpeerchannels](Msg::Timer10Minutes const& _) {
		return Boss::concurrent(do_listpeerchannels(false));
	});
}

ListpeersAnnouncer::ListpeersAnnouncer(S::Bus& bus_)
	: bus(bus_)
	, rpc(Util::make_unique<ModG::RpcProxy>(bus))
	{ start(); }

ListpeersAnnouncer::~ListpeersAnnouncer() =default;
}}
