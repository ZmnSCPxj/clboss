#include"Boss/Mod/FundsMover/Claimer.hpp"
#include"Boss/Mod/FundsMover/Main.hpp"
#include"Boss/Mod/FundsMover/PaymentDeleter.hpp"
#include"Boss/Mod/FundsMover/Runner.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/RequestMoveFunds.hpp"
#include"Boss/Msg/Timer10Minutes.hpp"
#include"Boss/concurrent.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"

namespace Boss { namespace Mod { namespace FundsMover {

class Main::Impl {
private:
	S::Bus& bus;
	Claimer claimer;
	Boss::Mod::Rpc* rpc;
	Ln::NodeId self_id;

	bool deleting_payments;

	void start() {
		bus.subscribe<Msg::Init>([this](Msg::Init const& init) {
			rpc = &init.rpc;
			self_id = init.self_id;
			return Boss::concurrent(delpay_our_payments());
		});
		bus.subscribe<Msg::RequestMoveFunds
			     >([this](Msg::RequestMoveFunds const& m) {
			auto msg = std::make_shared<Msg::RequestMoveFunds>(m);
			return wait_for_rpc().then([this, msg]() {
				auto runner = Runner::create( bus
							    , *rpc
							    , self_id
							    , claimer
							    , *msg
							    );
				return Runner::start(runner);
			});
		});
		bus.subscribe< Msg::Timer10Minutes
			     >([this](Msg::Timer10Minutes const&) {
			return wait_for_rpc() + delpay_our_payments();
		});
	}
	Ev::Io<void> wait_for_rpc() {
		return Ev::lift().then([this]() {
			if (!rpc)
				return Ev::yield() + wait_for_rpc();
			return Ev::lift();
		});
	}
	Ev::Io<void> delpay_our_payments() {
		return Ev::lift().then([this]() {
			if (deleting_payments)
				return Ev::lift();

			deleting_payments = true;
			return PaymentDeleter::run(bus, *rpc).then([this]() {
				deleting_payments = false;
				return Ev::lift();
			});
		});
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	explicit
	Impl(S::Bus& bus_) : bus(bus_)
			   , claimer(bus_)
			   , rpc(nullptr)
			   , deleting_payments(false)
			   { start(); }
};

Main::Main(Main&&) =default;
Main::~Main() =default;

Main::Main(S::Bus& bus) : pimpl(Util::make_unique<Impl>(bus)) { }

}}}
