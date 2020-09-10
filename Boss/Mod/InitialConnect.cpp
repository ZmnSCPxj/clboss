#include"Boss/Mod/InitialConnect.hpp"
#include"Boss/Msg/ListpeersResult.hpp"
#include"Boss/Msg/NeedsConnect.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include<assert.h>

namespace Boss { namespace Mod {

void InitialConnect::start() {
	bus.subscribe<Boss::Msg::ListpeersResult>([this](Boss::Msg::ListpeersResult const& lp) {
		if (lp.initial)
			return init_check(lp.result);
		else
			return periodic_check(lp.result);
	});
}

Ev::Io<void> InitialConnect::init_check(Jsmn::Object const& result) {
	if (result.is_object() && result.has("peers")) {
		auto peers = result["peers"];
		if (peers.is_array()) {
			for (size_t i = 0; i < peers.size(); ++i) {
				auto peer = peers[i];
				if (!(peer.is_object() && peer.has("channels")))
					continue;
				auto channels = peer["channels"];
				if (!channels.is_array())
					continue;
				for (size_t j = 0; j < channels.size(); ++j) {
					auto channel = channels[j];
					if (!(channel.is_object() && channel.has("state")))
						continue;
					auto state = channel["state"];
					if (!state.is_string())
						continue;
					auto state_s = std::string(state);
					if ( state_s == "CHANNELD_AWAITING_LOCKIN"
					  || state_s == "CHANNELD_NORMAL"
					  || state_s == "CHANNELD_SHUTTING_DOWN"
					   )
						return Ev::lift();
				}
			}
		}
	}

	/* If we got through all the peers and none of them
	 * are CHANNELD_*, then it is likely lightningd will
	 * not connect to anyone, so we should try adding
	 * more connections.  */
	return needs_connect("No peers with live channels.");
}

Ev::Io<void> InitialConnect::periodic_check(Jsmn::Object const& result) {
	if (result.is_object() && result.has("peers")) {
		auto peers = result["peers"];
		if (peers.is_array()) {
			for (size_t i = 0; i < peers.size(); ++i) {
				auto peer = peers[i];
				if (!(peer.is_object() && peer.has("connected")))
					continue;
				auto connected = peer["connected"];
				if ( connected.is_boolean()
				  && bool(connected)
				   )
					/* At least one connected peer.  */
					return Ev::lift();
			}
		}
	}

	/* If we got through all the peers and none of them
	 * are connected, we should probably try adding more
	 * connections.  */
	return needs_connect("No connected peers.");
}

Ev::Io<void> InitialConnect::needs_connect(std::string reason) {
	return Boss::log( bus, Debug
			, "InitialConnect: Needs connect: %s"
			, reason.c_str()
			).then([this]() {
		return Boss::concurrent(bus.raise(Boss::Msg::NeedsConnect()));
	});
}

}}
