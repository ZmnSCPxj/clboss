#include"Boss/Mod/RebalanceUnmanager.hpp"
#include"Boss/Msg/ProvideUnmanagement.hpp"
#include"Boss/Msg/RequestRebalanceUnmanaged.hpp"
#include"Boss/Msg/ResponseRebalanceUnmanaged.hpp"
#include"Boss/Msg/SolicitUnmanagement.hpp"
#include"Ev/Io.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<set>

namespace Boss { namespace Mod {

class RebalanceUnmanager::Impl {
private:
	S::Bus& bus;
	std::set<Ln::NodeId> unmanaged;

	void start() {
		bus.subscribe<Msg::SolicitUnmanagement
			     >([this](Msg::SolicitUnmanagement const&) {
			return bus.raise(Msg::ProvideUnmanagement{
				"balance",
				[this](Ln::NodeId const& n, bool unmanage) {
				if (unmanage) {
					unmanaged.insert(n);
				} else {
					auto it = unmanaged.find(n);
					if (it != unmanaged.end())
						unmanaged.erase(it);
				}
				return Ev::lift();
			}});
		});

		bus.subscribe<Msg::RequestRebalanceUnmanaged
			     >([this](Msg::RequestRebalanceUnmanaged const& m) {
			return bus.raise(Msg::ResponseRebalanceUnmanaged{
				m.requester, unmanaged
			});
		});
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	explicit
	Impl( S::Bus& bus_
	    , std::vector<std::string> un
	    ) : bus(bus_) {
		for (auto const& u : un)
			unmanaged.insert(Ln::NodeId(u));
		start();
	}
};

RebalanceUnmanager::RebalanceUnmanager(RebalanceUnmanager&&) =default;
RebalanceUnmanager::~RebalanceUnmanager() =default;

RebalanceUnmanager::RebalanceUnmanager(S::Bus& bus, std::vector<std::string> un)
	: pimpl(Util::make_unique<Impl>(bus, std::move(un))) { }

}}
