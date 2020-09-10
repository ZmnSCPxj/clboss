#include"Boss/Mod/ListpeersAnnouncer.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ListpeersResult.hpp"
#include"Boss/Msg/Timer10Minutes.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include<assert.h>

namespace Boss { namespace Mod {

void ListpeersAnnouncer::start() {
	auto do_listpeers = [this](bool initial) {
		assert(rpc);
		return rpc->command("listpeers"
				   , Json::Out().start_object().end_object()
				   ).then([ this
					  , initial
					  ](Jsmn::Object result) {
			return bus.raise(Msg::ListpeersResult{
				std::move(result), initial
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
