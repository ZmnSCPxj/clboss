#include"Boss/Mod/ChannelFeeSetter.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/AvailableRpcCommands.hpp"
#include"Boss/Msg/ProvideUnmanagement.hpp"
#include"Boss/Msg/SolicitUnmanagement.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/foreach.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include<inttypes.h>

namespace Boss { namespace Mod {

void ChannelFeeSetter::start() {
	have_setchannel = false;
	bus.subscribe<Msg::AvailableRpcCommands
		     >([this](Msg::AvailableRpcCommands const& m) {
		have_setchannel = m.commands.count("setchannel") != 0;
		return Ev::lift();
	});

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
	bus.subscribe< Msg::SolicitUnmanagement
		     >([this](Msg::SolicitUnmanagement const& _) {
		return bus.raise(Msg::ProvideUnmanagement{
			"lnfee", [this](Ln::NodeId const& n, bool flag) {
				if (flag)
					unmanaged.insert(n);
				else
					unmanaged.erase(unmanaged.find(n));
				return Ev::lift();
			}
		});
	});
}

Ev::Io<void> ChannelFeeSetter::set(Msg::SetChannelFee const& m) {
	if (unmanaged.count(m.node) != 0)
		return Boss::log( bus, Debug
				, "ChannelFeeSetter: %s not managed by \"lnfee\"; "
				  "would have set b=%" PRIu32 ", p=%" PRIu32 "."
				, std::string(m.node).c_str()
				, m.base
				, m.proportional
				);

	auto parms = Json::Out()
		.start_object()
			.field("id", std::string(m.node))
			.field(have_setchannel ? "feebase" : "base", m.base)
			.field(have_setchannel ? "feeppm" : "ppm", m.proportional)
		.end_object();
	return rpc->command(have_setchannel ? "setchannel" : "setchannelfee", std::move(parms)
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
