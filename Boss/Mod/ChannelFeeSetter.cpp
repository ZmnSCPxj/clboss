#include"Boss/Mod/ChannelFeeSetter.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/concurrent.hpp"
#include"Ev/Io.hpp"
#include"Ev/foreach.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"

namespace Boss { namespace Mod {

void ChannelFeeSetter::start() {
	bus.subscribe<Msg::Init
		     >([this](Msg::Init const& init) {
		rpc = &init.rpc;
		auto f = [this](Boss::Msg::SetChannelFee m) {
			return set(m);
		};
		return Boss::concurrent(
			Ev::foreach(f, std::move(pending))
		);
	});
	bus.subscribe<Msg::SetChannelFee
		     >([this](Msg::SetChannelFee const& m) {
		if (!rpc) {
			pending.push_back(m);
			return Ev::lift();
		}
		return set(m);
	});
}

Ev::Io<void> ChannelFeeSetter::set(Msg::SetChannelFee const& m) {
	auto parms = Json::Out()
		.start_object()
			.field("id", std::string(m.node))
			.field("base", m.base)
			.field("ppm", m.proportional)
		.end_object();
	return rpc->command("setchannelfee", std::move(parms)
			   ).then([](Jsmn::Object res) {
		return Ev::lift();
	}).catching<RpcError>([](RpcError const& _) {
		/* Ignore errors - there is race condition between
		 * when we think we still have a peer, and that
		 * peer closing the channel on us while we were
		 * doing processing that eventually triggers this
		 * module.
		 */
		return Ev::lift();
	});
}

}}
