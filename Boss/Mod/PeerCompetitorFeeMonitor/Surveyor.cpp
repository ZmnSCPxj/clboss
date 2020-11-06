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
		auto parms = Json::Out()
			.start_object()
				.field("source", std::string(peer_id))
			.end_object()
			;
		return rpc.command( "listchannels", std::move(parms));
	}).then([this](Jsmn::Object result) {
		try {
			auto cs = result["channels"];
			for (auto i = std::size_t(0); i < cs.size(); ++i) {
				auto c = cs[i];
				auto dest = Ln::NodeId(std::string(
					c["destination"]
				));
				/* Skip channels with us.  */
				if (dest == self_id)
					continue;
				to_process.push(Ln::Scid(std::string(
					c["short_channel_id"]
				)));
			}
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
		total_count = to_process.size();

		return loop();
	});
}

Ev::Io<void> Surveyor::loop() {
	if (to_process.empty())
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
	return std::move(act).then([this]() {
		auto scid = to_process.front();
		to_process.pop();
		++count;
		auto parms = Json::Out()
			.start_object()
				.field("short_channel_id", std::string(scid))
			.end_object()
			;
		return rpc.command("listchannels", std::move(parms));
	}).then([this](Jsmn::Object result) {
		try {
			auto cs = result["channels"];
			for (auto i = std::size_t(0); i < cs.size(); ++i) {
				auto c = cs[i];
				auto dest = Ln::NodeId(std::string(
					c["destination"]
				));
				/* We only care about the direction going
				 * *to* the target, because that is what
				 * we are competing against.
				 */
				if (dest != peer_id)
					continue;

				/* Sample the data.  */
				++samples;
				auto base = std::uint32_t(double(
					c["base_fee_millisatoshi"]
				));
				auto proportional = std::uint32_t(double(
					c["fee_per_millionth"]
				));
				auto weight = Ln::Amount(std::string(
					c["amount_msat"]
				));
				bases.add(base, weight);
				proportionals.add(proportional, weight);

				break;
			}
		} catch (Jsmn::TypeError const&) {
			auto os = std::ostringstream();
			os << result;
			return Boss::log( bus, Error
					, "PeerCompetitorFeeMonitor: "
					  "unexpected listchannels result: "
					  "%s"
					, os.str().c_str()
					);
		}

		return loop();
	});
}

}}}
