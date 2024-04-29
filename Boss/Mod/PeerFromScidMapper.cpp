#include"Boss/Mod/PeerFromScidMapper.hpp"
#include"Boss/Msg/ListpeersResult.hpp"
#include"Boss/Msg/RequestPeerFromScid.hpp"
#include"Boss/Msg/ResponsePeerFromScid.hpp"
#include"Ev/Io.hpp"
#include"Ln/NodeId.hpp"
#include"Ln/Scid.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<map>
#include<queue>

namespace Boss { namespace Mod {

class PeerFromScidMapper::Impl {
private:
	S::Bus& bus;

	std::unique_ptr<std::map<Ln::Scid, Ln::NodeId>> map;

	typedef
	std::queue<Msg::RequestPeerFromScid> PendingQ;
	PendingQ pendings;

	void start() {
		bus.subscribe<Msg::ListpeersResult
			     >([this](Msg::ListpeersResult const& m) {
			auto tmp = std::map<Ln::Scid, Ln::NodeId>();
			for (auto p : m.cpeers) {
				auto node = p.first;
				auto cs = p.second.channels;
				for (auto c : cs) {
					if (!c.has("short_channel_id"))
						continue;
					auto scid_j = c["short_channel_id"];
					if (!scid_j.is_string())
						continue;
					auto scid_s = std::string(scid_j);
					if (!Ln::Scid::valid_string(scid_s))
						continue;
					auto scid = Ln::Scid(scid_s);
					tmp[scid] = node;
				}
			}
			map = Util::make_unique<std::map< Ln::Scid
							, Ln::NodeId
							>>(std::move(tmp));
			auto ppendings = std::make_shared<PendingQ>(std::move(pendings));
			return resume_pendings(std::move(ppendings));
		});

		bus.subscribe<Msg::RequestPeerFromScid
			     >([this](Msg::RequestPeerFromScid const& m) {
			if (!map) {
				pendings.push(m);
				return Ev::lift();
			}

			auto it = map->find(m.scid);
			if (it == map->end()) {
				return bus.raise(Msg::ResponsePeerFromScid{
					m.requester, m.scid, Ln::NodeId()
				});
			}
			return bus.raise(Msg::ResponsePeerFromScid{
				m.requester, m.scid, it->second
			});
		});
	}

	Ev::Io<void>
	resume_pendings(std::shared_ptr<PendingQ> const& ppendings) {
		return Ev::lift().then([this, ppendings]() {
			if (ppendings->empty())
				return Ev::lift();
			auto pending = std::move(ppendings->front());
			ppendings->pop();
			return bus.raise(std::move(pending))
			     + resume_pendings(ppendings)
			     ;
		});
	}

public:
	Impl( S::Bus& bus_
	    ) : bus(bus_)
	      { start(); }
};

PeerFromScidMapper::PeerFromScidMapper(PeerFromScidMapper&&) =default;
PeerFromScidMapper::~PeerFromScidMapper() =default;

PeerFromScidMapper::PeerFromScidMapper(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
