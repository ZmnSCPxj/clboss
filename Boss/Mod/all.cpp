#include"Boss/Mod/CommandReceiver.hpp"
#include"Boss/Mod/JsonOutputter.hpp"
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Mod/all.hpp"
#include"Boss/Msg/Begin.hpp"
#include"Boss/Msg/JsonCin.hpp"
#include"Boss/Msg/JsonCout.hpp"
#include"Boss/concurrent.hpp"
#include"S/Bus.hpp"
#include<vector>

namespace {

class All {
private:
	std::vector<std::shared_ptr<void>> modules;

public:
	template<typename M, typename... As>
	std::shared_ptr<M> install(As&&... as) {
		auto ptr = std::make_shared<M>(as...);
		modules.push_back(std::shared_ptr<void>(ptr));
		return ptr;
	}
};

class Dummy {
private:
	S::Bus& bus;
	Boss::Mod::Waiter& waiter;

	Ev::Io<void> begin(Jsmn::Object const& inp) {
		return Ev::lift().then([this, inp]() {
			auto js = Json::Out()
				.start_object()
					.field("inp", inp)
				.end_object()
				;
			return bus.raise(Boss::Msg::JsonCout{js});
		});
	}

public:
	explicit Dummy( S::Bus& bus_
		      , Boss::Mod::Waiter& waiter_
		      ) : bus(bus_), waiter(waiter_) {
		bus.subscribe<Boss::Msg::JsonCin>([this](Boss::Msg::JsonCin const& inp) {
			return Boss::concurrent(begin(inp.obj));
		});
	}
};

}

namespace Boss { namespace Mod {

std::shared_ptr<void> all( std::ostream& cout
			 , S::Bus& bus
			 , Ev::ThreadPool& threadpool
			 ) {
	auto all = std::make_shared<All>();

	/* The waiter is shared among most of the other modules.  */
	auto waiter = all->install<Waiter>(bus);
	all->install<JsonOutputter>(cout, bus);
	all->install<CommandReceiver>(bus);

	(void) waiter;

	all->install<Dummy>(bus, *waiter);

	return all;
}

}}
