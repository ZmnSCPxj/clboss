#include"Boss/Mod/Waiter.hpp"
#include"Boss/Mod/all.hpp"
#include"Boss/Msg/Begin.hpp"
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
	std::ostream& cout;

	Ev::Io<void> begin() {
		return Ev::lift().then([this]() {
			return waiter.wait(5.0);
		}).then([this]() {
			cout << "Hello World" << std::endl;
			return Ev::lift();
		});
	}

public:
	explicit Dummy( S::Bus& bus_
		      , Boss::Mod::Waiter& waiter_
		      , std::ostream& cout_
		      ) : bus(bus_), waiter(waiter_), cout(cout_) {
		bus.subscribe<Boss::Msg::Begin>([this](Boss::Msg::Begin _) {
			return Boss::concurrent(begin());
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

	all->install<Dummy>(bus, *waiter, cout);

	return all;
}

}}
