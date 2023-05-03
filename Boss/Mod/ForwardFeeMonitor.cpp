#include"Boss/Mod/ForwardFeeMonitor.hpp"
#include"Boss/Msg/ForwardFee.hpp"
#include"Boss/Msg/ManifestNotification.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/Notification.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/map.hpp"
#include"Jsmn/Object.hpp"
#include"S/Bus.hpp"
#include"Util/stringify.hpp"

namespace Boss { namespace Mod {

void ForwardFeeMonitor::start() {
	bus.subscribe<Msg::Manifestation
		     >([this](Msg::Manifestation const& _) {
		return bus.raise(Msg::ManifestNotification{"forward_event"});
	});
	bus.subscribe<Msg::Notification
		     >([this](Msg::Notification const& n) {
		if (n.notification != "forward_event")
			return Ev::lift();

		auto in_scid = Ln::Scid();
		auto out_scid = Ln::Scid();
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

			in_scid = Ln::Scid(std::string(
				payload["in_channel"]
			));
			out_scid = Ln::Scid(std::string(
				payload["out_channel"]
			));
			fee = Ln::Amount::object(
				payload["fee_msat"]
			);
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

		auto f = [this](Ln::Scid scid) {
			return peer_from_scid_rr.execute(Msg::RequestPeerFromScid{
				nullptr, scid
			}).then([](Msg::ResponsePeerFromScid r) {
				return Ev::lift(std::move(r.peer));
			});
		};
		auto scids = std::vector<Ln::Scid>{in_scid, out_scid};
		return Ev::map( std::move(f), std::move(scids)
			      ).then([ this
				     , fee
				     , resolution_time
				     ](std::vector<Ln::NodeId> nids) {
			return cont( std::move(nids[0])
				   , std::move(nids[1])
				   , fee
				   , resolution_time
				   );
		});
	});
}

Ev::Io<void> ForwardFeeMonitor::cont( Ln::NodeId in_id
				    , Ln::NodeId out_id
				    , Ln::Amount fee
				    , double resolution_time
				    ) {
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
}

}}
