#include"Boss/Mod/FundsMover/PaymentDeleter.hpp"
#include"Boss/Mod/FundsMover/create_label.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Json/Out.hpp"
#include"Util/stringify.hpp"
#include<memory>

namespace Boss { namespace Mod { namespace FundsMover {

Ev::Io<void>
PaymentDeleter::run( S::Bus& bus
		   , Boss::Mod::Rpc& rpc
		   ) {
	auto self = std::shared_ptr<PaymentDeleter>(
		new PaymentDeleter(bus, rpc)
	);
	return self->core_run().then([self]() {
		return Ev::lift();
	});
}

Ev::Io<void>
PaymentDeleter::core_run() {
	return Ev::lift().then([this]() {
		return rpc.command("listpays", Json::Out::empty_object());
	}).then([this](Jsmn::Object res) {
		try {
			pays = res["pays"];
			it = pays.begin();
			count = 0;
		} catch (std::exception const&) {
			return Boss::log( bus, Error
					, "FundsMover: payment deleter: "
					  "Unexpected result from "
					  "`listpays`: %s"
					, Util::stringify(res).c_str()
					);
		}
		return loop();
	});
}

Ev::Io<void>
PaymentDeleter::loop() {
	return Ev::yield().then([this]() {
		if (it == pays.end())
			return Ev::lift();

		auto pay = *it;
		++it;
		try {
			if (!pay.has("label"))
				return loop();
			auto label = std::string(pay["label"]);
			if (!is_our_label(label))
				return loop();
			auto status = std::string(pay["status"]);
			if (status == "pending")
				return loop();
			auto payment_hash = std::string(pay["payment_hash"]);
			return Boss::log( bus, Debug
					, "FundsMover: payment deleter: "
					  "deleting %s payment with hash "
					  "%s."
					, status.c_str()
					, std::string(payment_hash).c_str()
					)
			     + delpay(payment_hash, status)
			     + loop()
			     ;
		} catch (std::exception const&) {
			return Boss::log( bus, Error
					, "FundsMover: payment deleter: "
					  "Unexpected `pays` entry from "
					  "`listpays`: %s"
					, Util::stringify(pay).c_str()
					);
		}
	});
}
Ev::Io<void>
PaymentDeleter::delpay( std::string const& payment_hash
		      , std::string const& status
		      ) {
	auto parms = Json::Out()
		.start_object()
			.field("payment_hash", payment_hash)
			.field("status", status)
		.end_object()
		;
	return rpc.command( "delpay"
			  , std::move(parms)
			  ).then([](Jsmn::Object res) {
		return Ev::lift();
	}).catching<RpcError>([](RpcError const&) {
		/* Ignore failures.  */
		return Ev::lift();
	});
}

}}}
