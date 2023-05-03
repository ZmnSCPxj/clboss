#include"Boss/Mod/PeerCompetitorFeeMonitor/Surveyor.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/now.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Util/make_unique.hpp"
#include<sstream>

namespace Boss { namespace Mod { namespace PeerCompetitorFeeMonitor {

Ev::Io<std::unique_ptr<Surveyor::Result>>
Surveyor::run() {
	auto self = shared_from_this();
	return self->core_run().then([self]() {
		return self->extract();
	});
}
Ev::Io<std::unique_ptr<Surveyor::Result>>
Surveyor::extract() {
	if (samples == 0)
		/* No data.  */
		return Ev::lift(std::unique_ptr<Result>());
	auto res = Util::make_unique<Result>();
	res->peer_id = std::move(peer_id);
	res->median_base = std::move(bases).finalize();
	res->median_proportional = std::move(proportionals).finalize();
	return Ev::lift(std::move(res));
}
Ev::Io<void> Surveyor::core_run() {
	return Ev::lift().then([this]() {
		return Boss::log( bus, Debug
				, "PeerCompetitorFeeMonitor: Surveying %s."
				, std::string(peer_id).c_str()
				);
	}).then([this]() {
		/* Older C-Lightning does not have `destination`
		 * parameter for `listchannels`.
		 */
		auto field = have_listchannels_destination ? "destination" : "source";
		auto parms = Json::Out()
			.start_object()
				.field(field, std::string(peer_id))
			.end_object()
			;
		return rpc.command( "listchannels", std::move(parms));
	}).then([this](Jsmn::Object result) {
		try {
			channels = result["channels"];
			if (!channels.is_array())
				throw Jsmn::TypeError();
			it = channels.begin();
		} catch (Jsmn::TypeError const&) {
			auto os = std::ostringstream();
			os << result;
			return Boss::log( bus, Error
					, "PeerCompetitorFeeMonitor: "
					  "unexpected listchannels result: "
					  "%s"
					, os.str().c_str()
					).then([]() {
				return Ev::lift(false);
			});
		}
		return Ev::lift(true);
	}).catching<RpcError>([this](RpcError const& e) {
		auto os = std::ostringstream();
		os << e.error;
		return Boss::log( bus, Error
				, "PeerCompetitorFeeMonitor: "
				  "listchannels failed: %s"
				, os.str().c_str()
				).then([]() {
			return Ev::lift(false);
		});
	}).then([this](bool ok) {
		if (!ok)
			return Ev::lift();

		prev_time = Ev::now();
		count = 0;
		total_count = channels.size();

		return loop();
	});
}

Ev::Io<void> Surveyor::loop() {
	if (it == channels.end())
		return Ev::lift();
	auto act = Ev::yield();
	if (Ev::now() - prev_time >= 5.0) {
		prev_time = Ev::now();
		act += Boss::log( bus, Info
				, "PeerCompetitorFeeMonitor: Surveying %s "
				  "progress: %zu/%zu (%f)"
				, std::string(peer_id).c_str()
				, count, total_count
				, double(count) / double(total_count)
				);
	}

	/* Back-compatibility mode check.  */
	if (!have_listchannels_destination)
		return std::move(act) + old_loop();

	return std::move(act).then([this]() {
		/* Process in batches for efficiency, then
		 * at the end of the batch, yield so that
		 * other greenthreads can run for a while.
		 */
		auto constexpr BATCH_SIZE = 50;
		for (auto i = 0; i < BATCH_SIZE && it != channels.end(); ++i) {
			auto c = *it;
			++it;
			auto err = one_channel(c);
			if (err != "")
				return Boss::log(bus, Error, "%s", err.c_str());
		}

		return loop();
	});
}

std::string Surveyor::one_channel(Jsmn::Object const& c) {
	++count;
	try {
		/* Skip our own channels with the
		 * peer.  */
		auto source = Ln::NodeId(std::string(
			c["source"]
		));
		if (source == self_id)
			return "";
		/* Sample the data.  */
		++samples;
		auto base = std::uint32_t(double(
			c["base_fee_millisatoshi"]
		));
		auto proportional = std::uint32_t(double(
			c["fee_per_millionth"]
		));
		auto weight = Ln::Amount::object(
			c["amount_msat"]
		);
		bases.add(base, weight);
		proportionals.add(proportional, weight);
	} catch (Jsmn::TypeError const&) {
		auto os = std::ostringstream();
		os << "PeerCompetitorFeeMonitor: "
		      "unexpected listchannels result: "
		   << c
		   ;
		return os.str();
	}
	return "";
}

Ev::Io<void> Surveyor::old_loop() {
	return Ev::lift().then([this]() {
		auto c =  *it;
		/* Channels has the wrong information: it
		 * specifies the fees *from* the peer node,
		 * but we want the fees *to* the peer node.
		 * So we need another listchannels command.
		 */
		auto scid = c["short_channel_id"];
		auto params = Json::Out()
			.start_object()
				.field("short_channel_id", scid)
			.end_object()
			;
		return rpc.command("listchannels", std::move(params));
	}).then([this](Jsmn::Object result) {
		/* This will return two channels, one in both
		 * directions.
		 * Get the one where destination is peer_id.
		 */
		for (auto c : result["channels"]) {
			auto destination = Ln::NodeId(std::string(
				c["destination"]
			));
			if (destination != peer_id)
				continue;

			auto err = one_channel(c);
			if (err != "")
				return Boss::log(bus, Error, "%s", err.c_str());

			/* If we reahed here, no need to
			 * check the other channel.
			 */
			break;
		}
		++it;
		return Ev::lift();
	}).catching<RpcError>([this](RpcError const& e) {
		return Boss::log( bus, Error
				, "PeerCompetitorFeeMonitor: "
				  "`listchannels` failed: %s"
				, e.error.direct_text().c_str()
				);
	}).catching<std::exception>([this](std::exception const& e) {
		return Boss::log( bus, Error
				, "PeerCompetitorFeeMonitor: "
				  "Unexpected result from "
				  "listchannels: %s: %s"
				, e.what()
				, (*it).direct_text().c_str()
				);
	}).then([this]() {
		return loop();
	});
}

}}}
