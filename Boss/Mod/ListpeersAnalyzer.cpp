#include"Boss/Mod/ListpeersAnalyzer.hpp"
#include"Boss/Msg/ListpeersAnalyzedResult.hpp"
#include"Boss/Msg/ListpeersResult.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"

namespace Boss { namespace Mod {

ListpeersAnalyzer::ListpeersAnalyzer(S::Bus& bus) {
	bus.subscribe< Msg::ListpeersResult
		     >([&bus](Msg::ListpeersResult const& l) {
		auto ar = Msg::ListpeersAnalyzedResult();
		ar.initial = l.initial;

		for (auto i = std::size_t(0); i < l.peers.size(); ++i) {
			auto peer = l.peers[i];
			if (!peer.is_object() || !peer.has("id"))
				continue;
			if (!peer.has("connected"))
				continue;
			if (!peer.has("channels"))
				continue;

			auto id_j = peer["id"];
			if (!id_j.is_string())
				continue;
			auto id_s = std::string(id_j);
			if (!Ln::NodeId::valid_string(id_s))
				continue;
			auto id = Ln::NodeId(id_s);

			auto connected_j = peer["connected"];
			if (!connected_j.is_boolean())
				continue;
			auto connected = bool(connected_j);

			auto has_chan = bool(false);
			auto chans = peer["channels"];
			if (!chans.is_array())
				continue;
			for (auto j = std::size_t(0); j < chans.size(); ++j) {
				auto chan = chans[j];
				if (!chan.is_object() || !chan.has("state"))
					continue;

				auto state_j = chan["state"];
				if (!state_j.is_string())
					continue;
				auto state = std::string(state_j);
				auto prefix = std::string( state.begin()
							 , state.begin() + 8
							 );

				if ( prefix == "OPENINGD"
				  || prefix == "CHANNELD"
				   ) {
					has_chan = true;
					break;
				}
			}

			if (connected) {
				if (has_chan)
					ar.connected_channeled.emplace(id);
				else
					ar.connected_unchanneled.emplace(id);
			} else {
				if (has_chan)
					ar.disconnected_channeled.emplace(id);
				else
					ar.disconnected_unchanneled.emplace(id);
			}
		}

		return bus.raise(std::move(ar));
	});
}

}}
