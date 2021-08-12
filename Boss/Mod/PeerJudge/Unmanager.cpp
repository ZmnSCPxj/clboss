#include"Boss/Mod/PeerJudge/Unmanager.hpp"
#include"Boss/Msg/ProvideUnmanagement.hpp"
#include"Boss/Msg/SolicitUnmanagement.hpp"
#include"Ev/Io.hpp"
#include"S/Bus.hpp"
#include<utility>

namespace Boss { namespace Mod { namespace PeerJudge {

Unmanager::Unmanager(S::Bus& bus) {
	bus.subscribe<Msg::SolicitUnmanagement
		     > ([this, &bus](Msg::SolicitUnmanagement const& _) {
		auto fun = [this](Ln::NodeId const& node, bool is_unmanaged) {
			if (is_unmanaged)
				unmanaged.insert(node);
			else
				unmanaged.erase(unmanaged.find(node));
			return Ev::lift();
		};
		return bus.raise(Msg::ProvideUnmanagement{
			"close",
			std::move(fun)
		});
	});
}

}}}

