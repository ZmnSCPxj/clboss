#include"Boss/Mod/JsonOutputter.hpp"
#include"Boss/Msg/JsonCout.hpp"
#include"Boss/concurrent.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"S/Bus.hpp"

namespace Boss { namespace Mod {

JsonOutputter::JsonOutputter( std::ostream& cout_
			    , S::Bus& bus
			    ) : cout(cout_) {
	bus.subscribe<Boss::Msg::JsonCout>([this](Boss::Msg::JsonCout const& j) {
		auto& obj = j.obj;
		auto start = outs.empty();

		outs.push(obj.output());

		if (!start)
			return Ev::lift();
		return Boss::concurrent(loop());
	});
}

Ev::Io<void> JsonOutputter::loop() {
	return Ev::yield().then([this]() {
		if (outs.empty())
			return Ev::lift();
		cout << outs.front() << std::endl;
		outs.pop();
		return loop();
	});
}

}}
