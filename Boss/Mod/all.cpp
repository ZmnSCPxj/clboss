#include"Boss/Mod/BlockTracker.hpp"
#include"Boss/Mod/Connector.hpp"
#include"Boss/Mod/CommandReceiver.hpp"
#include"Boss/Mod/InitialConnect.hpp"
#include"Boss/Mod/Initiator.hpp"
#include"Boss/Mod/JsonOutputter.hpp"
#include"Boss/Mod/Manifester.hpp"
#include"Boss/Mod/Timers.hpp"
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Mod/all.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/ManifestNotification.hpp"
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

	void start() {
		bus.subscribe<Boss::Msg::Manifestation>([this](Boss::Msg::Manifestation const& _) {
			return bus.raise(Boss::Msg::ManifestNotification{"forward_event"});
		});
	}

public:
	explicit Dummy( S::Bus& bus_) : bus(bus_) {
		start();
	}
};

}

namespace Boss { namespace Mod {

std::shared_ptr<void> all( std::ostream& cout
			 , S::Bus& bus
			 , Ev::ThreadPool& threadpool
			 , std::function< Net::Fd( std::string const&
						 , std::string const&
						 )
					> open_rpc_socket
			 ) {
	auto all = std::make_shared<All>();

	/* The waiter is shared among most of the other modules.  */
	auto waiter = all->install<Waiter>(bus);
	all->install<JsonOutputter>(cout, bus);
	all->install<CommandReceiver>(bus);
	all->install<Manifester>(bus);
	all->install<Initiator>(bus, threadpool, std::move(open_rpc_socket));
	all->install<BlockTracker>(bus);
	all->install<Timers>(bus, *waiter);
	all->install<InitialConnect>(bus);
	all->install<Connector>(bus);

	all->install<Dummy>(bus);

	return all;
}

}}
