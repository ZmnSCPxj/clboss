#include"Boss/Mod/PeerComplaintsDesk/Main.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"

namespace Boss { namespace Mod { namespace PeerComplaintsDesk {

class Main::Impl {
private:
	S::Bus& bus;

	void start() {
	}

public:
	Impl() =delete;
	Impl(Impl const&) =delete;
	Impl(Impl&&) =delete;

	explicit
	Impl(S::Bus& bus_) : bus(bus_) { start(); }
};

Main::Main(Main&&) =default;
Main::~Main() =default;

Main::Main(S::Bus& bus) : pimpl(Util::make_unique<Impl>(bus)) { }

}}}
