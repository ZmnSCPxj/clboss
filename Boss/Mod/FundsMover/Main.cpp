#include"Boss/Mod/FundsMover/Claimer.hpp"
#include"Boss/Mod/FundsMover/Main.hpp"
#include"Boss/Mod/FundsMover/Runner.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/RequestMoveFunds.hpp"
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

	void start() {
		bus.subscribe<Msg::Init>([this](Msg::Init const& init) {
			rpc = &init.rpc;
			self_id = init.self_id;
			return Ev::lift();
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
	}
	Ev::Io<void> wait_for_rpc() {
		return Ev::lift().then([this]() {
			if (!rpc)
				return Ev::yield() + wait_for_rpc();
			return Ev::lift();
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
			   { start(); }
};

Main::Main(Main&&) =default;
Main::~Main() =default;

Main::Main(S::Bus& bus) : pimpl(Util::make_unique<Impl>(bus)) { }

}}}
