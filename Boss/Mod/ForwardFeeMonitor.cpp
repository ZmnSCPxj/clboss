#include"Boss/Mod/ForwardFeeMonitor.hpp"
#include"Boss/Msg/ForwardFee.hpp"
#include"Boss/Msg/ListpeersResult.hpp"
#include"Boss/Msg/ManifestNotification.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/Notification.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Jsmn/Object.hpp"
#include"S/Bus.hpp"
#include"Util/stringify.hpp"

namespace Boss { namespace Mod {

void ForwardFeeMonitor::start() {
	bus.subscribe<Msg::Manifestation
		     >([this](Msg::Manifestation const& _) {
		return bus.raise(Msg::ManifestNotification{"forward_event"});
	});
	bus.subscribe<Msg::ListpeersResult
		     >([this](Msg::ListpeersResult const& r) {
		/* Construct-temporary...*/
		auto tmp = std::map<Ln::Scid, Ln::NodeId>();
		try {
			for ( auto i = std::size_t(0)
			    ; i < r.peers.size()
			    ; ++i
			    ) {
				auto peer = r.peers[i];
				auto peerid = Ln::NodeId(std::string(
					peer["id"]
				));
				auto cs = peer["channels"];
				for ( auto j = std::size_t(0)
				    ; j < cs.size()
				    ; ++j
				    ) {
					auto c = cs[j];
					if (!c.has("short_channel_id"))
						continue;
					auto scid = Ln::Scid(std::string(
						c["short_channel_id"]
					));
					tmp[scid] = peerid;
				}
			}
		} catch (std::runtime_error const& _) {
			return Boss::log( bus, Error
					, "ForwardFeeMonitor: Unexpected "
					  "listpeers result: %s"
					, Util::stringify(r.peers).c_str()
					);
		}
		/* ...and-swap.  */
		peers = std::move(tmp);
		return Ev::lift();
	});

	bus.subscribe<Msg::Notification
		     >([this](Msg::Notification const& n) {
		if (n.notification != "forward_event")
			return Ev::lift();

		auto in_id = Ln::NodeId();
		auto out_id = Ln::NodeId();
		auto fee = Ln::Amount();
		auto resolution_time = double();
		try {
			auto payload = n.params["forward_event"];
			if ( !payload.has("out_channel")
			  || !payload.has("fee")
			  || !payload.has("resolved_time")
			  || !payload.has("received_time")
			   )
				return Ev::lift();
			if (std::string(payload["status"]) != "settled")
				return Ev::lift();

			auto in_scid = Ln::Scid(std::string(
				payload["in_channel"]
			));
			auto out_scid = Ln::Scid(std::string(
				payload["out_channel"]
			));
			/* Map SCID to node ID.  */
			auto in_it = peers.find(in_scid);
			if (in_it != peers.end())
				in_id = in_it->second;
			auto out_it = peers.find(out_scid);
			if (out_it != peers.end())
				out_id = out_it->second;

			fee = Ln::Amount(std::string(
				payload["fee_msat"]
			));
			resolution_time = double(payload["resolved_time"])
					- double(payload["received_time"])
					;

		} catch (std::runtime_error const& _) {
			return Boss::log( bus, Error
					, "ForwardFeeMonitor: Unexpected "
					  "forward_event payload: %s"
					, Util::stringify(n.params).c_str()
					);
		}

		if (!in_id || !out_id)
			return Ev::lift();

		auto act = Ev::lift();
		act += Boss::log( bus, Debug
				, "ForwardFeeMonitor: %s -> %s, fee: %s, "
				  "time: %.3f"
				, std::string(in_id).c_str()
				, std::string(out_id).c_str()
				, std::string(fee).c_str()
				, resolution_time
				);
		act += Boss::concurrent(bus.raise(Msg::ForwardFee{
			std::move(in_id),
			std::move(out_id),
			fee,
			resolution_time
		}));
		return act;
	});
}

}}
