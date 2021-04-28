#include"Boss/Mod/PaymentDeleter.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ProvideDeletablePaymentLabelFilter.hpp"
#include"Boss/Msg/SolicitDeletablePaymentLabelFilter.hpp"
#include"Boss/Msg/TimerRandomDaily.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"
#include<vector>

namespace Boss { namespace Mod {

class PaymentDeleter::Impl {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;

	bool running;
	bool soliciting;
	std::vector<std::function<bool(std::string const&)>> filters;

	Jsmn::Object pays;
	Jsmn::Object::const_iterator it;

	void start() {
		rpc = nullptr;
		running = false;
		soliciting = false;
		filters.clear();


		bus.subscribe<Msg::Init
			     >([this](Msg::Init const& init) {
			rpc = &init.rpc;

			return initialize();
		});
		bus.subscribe<Msg::TimerRandomDaily
			     >([this](Msg::TimerRandomDaily const& _) {
			return Boss::concurrent(perform());
		});
		bus.subscribe<Msg::ProvideDeletablePaymentLabelFilter
			     >([this
			       ](Msg::ProvideDeletablePaymentLabelFilter const& m) {
			if (!soliciting)
				return Ev::lift();
			filters.push_back(m.filter);
			return Ev::lift();
		});
	}

	Ev::Io<void> initialize() {
		using Msg::SolicitDeletablePaymentLabelFilter;
		return Ev::lift().then([this]() {
			/* Solicit filter functions.  */
			soliciting = true;
			return bus.raise(
				SolicitDeletablePaymentLabelFilter{}
			);
		}).then([this]() {
			soliciting = false;

			/* Unclean shutdown is the most likely reason to
			 * have dangling failed payments, so this is the
			 * best time to perform cleanup.
			 */
			return Boss::concurrent(perform());
		});
	}

	Ev::Io<void> perform() {
		return Ev::lift().then([this]() {
			if (running)
				return Ev::lift();

			running = true;
			return core_perform().then([this]() {
				running = false;

				return Ev::lift();
			});
		});
	}

	bool filter_label(std::string const& label) {
		for (auto const& f : filters)
			if (f(label))
				return true;
		return false;
	}

	Ev::Io<void> core_perform() {
		return Ev::lift().then([this]() {
			return Boss::log( bus, Debug
					, "PaymentDeleter: Start."
					);
		}).then([this]() {

			return rpc->command("listpays", Json::Out::empty_object());
		}).then([this](Jsmn::Object res) {
			try {
				pays = res["pays"];
				it = pays.begin();
			} catch (std::exception const&) {
				return Boss::log( bus, Error
						, "PaymentDeleter: Unexpected "
						  "result from 'listpays': "
						  "%s"
						, Util::stringify(res).c_str()
						);
			}
			return loop();
		}).then([this]() {
			return Boss::log( bus, Debug
					, "PaymentDeleter: End."
					);
		});
	}
	Ev::Io<void> loop() {
		return Ev::yield().then([this]() {
			if (it == pays.end())
				return Ev::lift();

			auto pay = *it;
			++it;
			try {
				/* All our payments must be labelled.  */
				if (!pay.has("label"))
					return loop();
				/* Is it one of ours?  */
				auto label = std::string(pay["label"]);
				if (!filter_label(label))
					return loop();
				/* If pending, skip.  */
				auto status = std::string(pay["status"]);
				if (status == "pending")
					return loop();

				auto payment_hash = std::string(
					pay["payment_hash"]
				);
				return Boss::log( bus, Debug
						, "PaymentDeleter: "
						  "Deleting %s payment with "
						  "hash %s."
						, status.c_str()
						, payment_hash.c_str()
						)
				     + delpay(payment_hash, status)
				     + loop()
				     ;
			} catch (std::exception const&) {
				return Boss::log( bus, Error
						, "PaymentDeleter: "
						  "Unexpected 'pays' entry "
						  "from 'listpays': %s"
						, Util::stringify(pay)
							.c_str()
						);
			}
		});
	}

	Ev::Io<void>
	delpay( std::string const& payment_hash
	      , std::string const& status
	      ) {
		auto parms = Json::Out()
			.start_object()
				.field("payment_hash", payment_hash)
				.field("status", status)
			.end_object()
			;
		return rpc->command( "delpay"
				   , std::move(parms)
				   ).then([](Jsmn::Object _) {
			/* Ignore result.  */
			return Ev::lift();
		}).catching<RpcError>([](RpcError const& _) {
			/* Ignore failure.  */
			return Ev::lift();
		});
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;

	Impl( S::Bus& bus_
	    ) : bus(bus_)
	      { start(); }
};

PaymentDeleter::~PaymentDeleter() =default;
PaymentDeleter::PaymentDeleter(PaymentDeleter&&) =default;

PaymentDeleter::PaymentDeleter(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
