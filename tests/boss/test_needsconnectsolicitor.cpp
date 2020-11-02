#undef NDEBUG
#include"Boss/Mod/NeedsConnectSolicitor.hpp"
#include"Boss/Msg/NeedsConnect.hpp"
#include"Boss/Msg/ProposeConnectCandidates.hpp"
#include"Boss/Msg/RequestConnect.hpp"
#include"Boss/Msg/ResponseConnect.hpp"
#include"Boss/Msg/SolicitConnectCandidates.hpp"
#include"Boss/Msg/TaskCompletion.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"S/Bus.hpp"
#include<assert.h>
#include<functional>
#include<queue>
#include<string>
#include<vector>

namespace {

class DummyConnector {
public:
	/* What to return in Boss::Msg::ResponseConnect.  */
	bool success;
	/* How many times have we gotten RequestConnect.  */
	size_t connects;

	DummyConnector(S::Bus& bus) {
		success = true;
		connects = 0;
		bus.subscribe<Boss::Msg::RequestConnect>([this, &bus](Boss::Msg::RequestConnect const& req) {
			++connects;
			return bus.raise(Boss::Msg::ResponseConnect{req.node, success});
		});
	}
	DummyConnector(DummyConnector&&) =delete;
};

class DummyConnectDiscoverer {
public:
	std::vector<std::string> to_return;

	DummyConnectDiscoverer(S::Bus& bus) {
		bus.subscribe<Boss::Msg::SolicitConnectCandidates>([this, &bus](Boss::Msg::SolicitConnectCandidates const& _) {
			if (to_return.empty())
				return Ev::lift();
			return bus.raise(Boss::Msg::ProposeConnectCandidates{
				std::move(to_return)
			});
		});
	}
	DummyConnectDiscoverer(DummyConnectDiscoverer&&) =delete;
};
class TaskCompletionWaiter {
private:
	size_t pending_completions;
	std::queue<std::function<void()>> passes;

	void trigger() {
		while (pending_completions && !passes.empty()) {
			auto pass = std::move(passes.front());
			passes.pop();
			--pending_completions;
			pass();
		}
	}
	

public:
	TaskCompletionWaiter(S::Bus& bus
			    ) : pending_completions(0)
			      , passes()
			      {
		bus.subscribe<Boss::Msg::TaskCompletion>([this](Boss::Msg::TaskCompletion const& _) {
			++pending_completions;
			trigger();
			return Ev::lift();
		});
	}
	TaskCompletionWaiter(TaskCompletionWaiter&&) =delete;

	Ev::Io<void> wait() {
		return Ev::Io<void>([this
				    ]( std::function<void()> pass
				     , std::function<void(std::exception_ptr)> fail
				     ) {
			passes.emplace(std::move(pass));
			trigger();
		});
	}
};

}

int main() {
	S::Bus bus;
	DummyConnector connector(bus);
	DummyConnectDiscoverer discoverer(bus);
	TaskCompletionWaiter waiter(bus);
	Boss::Mod::NeedsConnectSolicitor solicitor(bus);


	auto needs_connect = [&bus, &waiter]() {
		return bus.raise(Boss::Msg::NeedsConnect()).then([&waiter]() {
			return waiter.wait();
		});
	};

	auto test = Ev::lift().then([&]() {
		connector.success = true;

		/* Nothing to connect to.  */
		discoverer.to_return.clear();
		return needs_connect();
	}).then([&]() {
		/* Since there is nothing to connect, there should not
		 * have been any connection attempts.  */
		assert(connector.connects == 0);

		/* Now add some things to connect.  */
		discoverer.to_return.push_back("1");
		discoverer.to_return.push_back("2");
		discoverer.to_return.push_back("3");
		discoverer.to_return.push_back("4");
		discoverer.to_return.push_back("5");
		return needs_connect();
	}).then([&]() {
		/* Since the dummy connector is in "success" mode,
		 * there should have been at most two connect attempts,
		 * but more than 0.  */
		assert(connector.connects > 0);
		assert(connector.connects <= 2);

		/* Reset, then make connections fail.  */
		connector.connects = 0;
		connector.success = false;
		/* Add some things to connect.  */
		discoverer.to_return.push_back("1");
		discoverer.to_return.push_back("2");
		discoverer.to_return.push_back("3");
		discoverer.to_return.push_back("4");
		discoverer.to_return.push_back("5");
		return needs_connect();
	}).then([&]() {
		/* The dummy connector failed them all, so
		 * there should be 5 connect attempts.  */
		assert(connector.connects == 5);

		return Ev::lift(0);
	});

	return Ev::start(test);
}
