#include"Boss/Mod/CircularRebalanceController.hpp"
#include"Boss/ModG/TimedBoolean.hpp"
#include"Boss/Msg/RequestGetCircularRebalanceFlag.hpp"
#include"Boss/Msg/ResponseGetCircularRebalanceFlag.hpp"

namespace Boss { namespace Mod {

class CircularRebalanceController::Impl {
private:
	Boss::ModG::TimedBoolean<Msg::RequestGetCircularRebalanceFlag, Msg::ResponseGetCircularRebalanceFlag> timed_boolean;

	void start() {
		timed_boolean.start();
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	Impl(S::Bus& bus_)
	: timed_boolean(bus_, "CircularRebalanceController", "circular-rebalance",
			"circular channel rebalancing") { start(); }
};

CircularRebalanceController::CircularRebalanceController(CircularRebalanceController&&) =default;
CircularRebalanceController::~CircularRebalanceController() =default;

CircularRebalanceController::CircularRebalanceController(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
