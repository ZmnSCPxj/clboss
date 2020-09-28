#include"Boss/Mod/ListpaysHandler.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/RequestListpays.hpp"
#include"Boss/Msg/ResponseListpays.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/foreach.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include<sstream>

namespace Boss { namespace Mod {

void ListpaysHandler::start() {
	bus.subscribe< Msg::Init
		     >([this](Msg::Init const& init) {
		rpc = &init.rpc;
		auto f = [this](Sha256::Hash h) { return listpays(h); };
		return Boss::concurrent(
			Ev::foreach(f, std::move(pending))
		);
	});
	bus.subscribe< Msg::RequestListpays
		     >([this](Msg::RequestListpays const& m) {
		if (!rpc) {
			pending.push_back(m.payment_hash);
			return Ev::lift();
		}
		return Boss::concurrent(listpays(m.payment_hash));
	});
}

Ev::Io<void> ListpaysHandler::listpays(Sha256::Hash h) {
	return Ev::lift().then([this, h]() {
		auto parms = Json::Out()
			.start_object()
				.field("payment_hash", std::string(h))
			.end_object()
			;
		return rpc->command("listpays", std::move(parms));
	}).then([this, h](Jsmn::Object res) {
		auto resp = Msg::ResponseListpays();
		resp.payment_hash = h;

		try {
			auto pays = res["pays"];
			if (pays.size() == 0) {
				resp.status = Msg::StatusListpays_nonexistent;
				return Ev::lift(resp);
			}
			auto pay = pays[0];

			auto payee_s = std::string(pay["destination"]);
			resp.payee = Ln::NodeId(payee_s);

			auto status = std::string(pay["status"]);
			if (status == "failed") {
				resp.status = Msg::StatusListpays_failed;
			} else if (status == "pending") {
				resp.status = Msg::StatusListpays_pending;
			} else if (status == "complete") {
				resp.status = Msg::StatusListpays_success;
				resp.amount = Ln::Amount(std::string(
					pay["amount_msat"]
				));
				resp.amount_sent = Ln::Amount(std::string(
					pay["amount_sent_msat"]
				));
			} else throw Jsmn::TypeError();
		} catch (Jsmn::TypeError const&) {
			resp.status = Msg::StatusListpays_nonexistent;

			auto os = std::ostringstream();
			os << res;
			return Boss::log( bus, Error
					, "ListpaysHandler: `listpays %s` "
					  "had unexpected result %s"
					, std::string(h).c_str()
					, os.str().c_str()
					).then([resp]() {
				return Ev::lift(resp);
			});
		}

		return Ev::lift(std::move(resp));
	}).catching<RpcError>([this, h](RpcError const&) {
		return Boss::log( bus, Error
				, "ListpaysHandler: `listpays %s` failed"
				, std::string(h).c_str()
				).then([h]() {
			auto err = Msg::ResponseListpays{
				h, Msg::StatusListpays_nonexistent,

				Ln::NodeId(),

				Ln::Amount(), Ln::Amount()
			};
			return Ev::lift(std::move(err));
		});
	}).then([this](Msg::ResponseListpays resp) {
		return bus.raise(std::move(resp));
	});
}

}}
