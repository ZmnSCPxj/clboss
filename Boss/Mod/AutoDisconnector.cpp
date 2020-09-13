#include"Boss/Mod/AutoDisconnector.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ListpeersAnalyzedResult.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
#include"Ev/map.hpp"
#include"Json/Out.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include<algorithm>
#include<cstddef>
#include<functional>
#include<iterator>
#include<sstream>
#include<vector>

namespace {

/* Number of connections max with no channels.  */
auto const max_unchanneled_connections = std::size_t(3);

}

namespace Boss { namespace Mod {

void AutoDisconnector::start() {
	using std::placeholders::_1;
	typedef AutoDisconnector This;
	bus.subscribe< Msg::Init
		     >(std::bind(&This::on_init, this, _1));
	bus.subscribe< Msg::ListpeersAnalyzedResult
		     >(std::bind(&This::on_listpeers, this, _1));
}
Ev::Io<void> AutoDisconnector::on_init(Msg::Init const& init) {
	rpc = &init.rpc;
	return Ev::lift();
}
Ev::Io<void>
AutoDisconnector::on_listpeers(Msg::ListpeersAnalyzedResult const& l) {
	if (l.connected_unchanneled.size() <= max_unchanneled_connections)
		return Ev::lift();

	/* Copy the nodes.  */
	auto nodes = std::make_shared<std::vector<Ln::NodeId>>();
	std::copy( l.connected_unchanneled.begin()
		 , l.connected_unchanneled.end()
		 , std::back_inserter(*nodes)
		 );
	/* Shuffle, then remove the last few nodes.  */
	std::shuffle(nodes->begin(), nodes->end(), Boss::random_engine);
	nodes->erase( nodes->end() - max_unchanneled_connections
		    , nodes->end()
		    );

	auto first = true;
	auto os = std::ostringstream();
	for (auto const& n : *nodes) {
		if (first)
			first = false;
		else
			os << ", ";
		os << n;
	}

	return Boss::log( bus, Info
			, "AutoDisconnector: %s"
			, os.str().c_str()
			).then([this, nodes]() {
		/* Create disconnect function.  */
		auto disconnect = [this](Ln::NodeId n) {
			auto n_s = std::string(n);
			return rpc->command( "disconnect"
					   , Json::Out()
						.start_object()
							.field("id", n_s)
						.end_object()
					   ).then([](Jsmn::Object _) {
				return Ev::lift(0);
			}).catching<RpcError>([](RpcError const& _) {
				return Ev::lift(0);
			});
		};
		/* Run disconnect in parallel.  */
		return Ev::map(disconnect, std::move(*nodes));
	}).then([](std::vector<int> _) {
		return Ev::lift();
	});
}

}}
