#include"Boss/Mod/AutoSwapController.hpp"
#include"Boss/ModG/TimedBoolean.hpp"
#include"Boss/Msg/RequestGetAutoSwapFlag.hpp"
#include"Boss/Msg/ResponseGetAutoSwapFlag.hpp"

namespace Boss { namespace Mod {

class AutoSwapController::Impl {
private:
	Boss::ModG::TimedBoolean<Msg::RequestGetAutoSwapFlag
				, Msg::ResponseGetAutoSwapFlag> timed_boolean;

	void start() {
		timed_boolean.start();
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	Impl(S::Bus& bus_)
	: timed_boolean(bus_, "AutoSwapController", "auto-swap",
			"automatic offchain-to-onchain swap") { start(); }
};

AutoSwapController::AutoSwapController(AutoSwapController&&) =default;
AutoSwapController::~AutoSwapController() =default;

AutoSwapController::AutoSwapController(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
