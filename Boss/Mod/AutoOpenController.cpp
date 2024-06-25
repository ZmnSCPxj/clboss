#include"Boss/Mod/AutoOpenController.hpp"
#include"Boss/ModG/TimedBoolean.hpp"
#include"Boss/Msg/RequestGetAutoOpenFlag.hpp"
#include"Boss/Msg/ResponseGetAutoOpenFlag.hpp"

namespace Boss { namespace Mod {

class AutoOpenController::Impl {
private:
	Boss::ModG::TimedBoolean<Msg::RequestGetAutoOpenFlag, Msg::ResponseGetAutoOpenFlag> timed_boolean;

	void start() {
		timed_boolean.start();
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	Impl(S::Bus& bus_)
	: timed_boolean(bus_, "AutoOpenController", "auto-open-channels",
			"automatic channel opening") { start(); }
};

AutoOpenController::AutoOpenController(AutoOpenController&&) =default;
AutoOpenController::~AutoOpenController() =default;

AutoOpenController::AutoOpenController(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
