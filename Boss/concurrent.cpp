#include"Boss/Shutdown.hpp"
#include"Boss/concurrent.hpp"
#include"Ev/Io.hpp"
#include"Ev/concurrent.hpp"

namespace Boss {

Ev::Io<void> concurrent(Ev::Io<void> io) {
	return Ev::concurrent(std::move(io).catching<Boss::Shutdown>([](Boss::Shutdown _) {
		return Ev::lift();
	}));
}

}
