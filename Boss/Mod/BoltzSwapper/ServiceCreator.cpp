#include"Boltz/Service.hpp"
#include"Boltz/ServiceFactory.hpp"
#include"Boss/Mod/BoltzSwapper/ServiceCreator.hpp"
#include"Boss/Mod/BoltzSwapper/ServiceModule.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/Network.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<queue>
#include<sstream>

namespace Boss { namespace Mod { namespace BoltzSwapper {

class ServiceCreator::Core : public std::enable_shared_from_this<Core> {
private:
	S::Bus& bus;
	Boltz::ServiceFactory factory;
	std::queue<Instance> instances;
	std::vector<std::unique_ptr<ServiceModule>>& services;

	bool first;
	std::ostringstream report;

	Core( S::Bus& bus_
	    , Boltz::ServiceFactory factory_
	    , std::queue<Instance> instances_
	    , std::vector<std::unique_ptr<ServiceModule>>& services_
	    ) : bus(bus_)
	      , factory(std::move(factory_))
	      , instances(std::move(instances_))
	      , services(services_)
	      { }
public:
	static
	std::shared_ptr<Core>
	create( S::Bus& bus
	      , Boltz::ServiceFactory factory
	      , std::queue<Instance> instances
	      , std::vector<std::unique_ptr<ServiceModule>>& services
	      ) {
		return std::shared_ptr<Core>(
			new Core( bus
				, std::move(factory)
				, std::move(instances)
				, services
				)
		);
	}

	Ev::Io<void> run() {
		auto self = shared_from_this();
		self->first = true;
		return self->core_run().then([self]() {
			return Ev::lift();
		});
	}

private:
	Ev::Io<void> core_run() {
		return Ev::yield().then([this]() {
			if (instances.empty()) {
				return Boss::log( bus, Debug
						, "BoltzSwapper: Created: %s"
						, report.str().c_str()
						);
			}

			auto instance = std::move(instances.front());
			instances.pop();
			if (first)
				first = false;
			else
				report << ", ";
			report << "Boltz::Service(\"" << instance.label << "\")";

			return factory.create_service_detailed(
				instance.label, instance.clearnet, instance.onion
			).then([this](std::unique_ptr<Boltz::Service> core) {
				auto wrapper = Util::make_unique<ServiceModule>
					( bus, std::move(core) );
				services.push_back(std::move(wrapper));
				return core_run();
			});
		});
	}
};

void ServiceCreator::start() {
	bus.subscribe<Msg::Init>([this](Msg::Init const& init) {
		auto it = instances.find(init.network);
		if (it == instances.end())
			return Boss::log( bus, Debug
					, "BoltzSwapper: "
					  "No instances for this network."
					);

		auto factory = Boltz::ServiceFactory
			( threadpool
			, init.db
			, init.signer
			, env
			, init.proxy
			, init.always_use_proxy
			);
		auto q = std::queue<Instance>();
		for (auto const& instance : it->second)
			q.push(instance);
		auto core = Core::create( bus
					, std::move(factory)
					, std::move(q)
					, services
					);
		return Boss::concurrent(core->run());
	});
}

}}}
