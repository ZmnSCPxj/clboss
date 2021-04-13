#include"Boss/Mod/ConnectFinderByHardcode.hpp"
#include"Boss/Msg/ProposeConnectCandidates.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/Network.hpp"
#include"Boss/Msg/SolicitConnectCandidates.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<map>
#include<string>
#include<vector>

namespace {

auto const all_nodes =
		std::map<Boss::Msg::Network, std::vector<std::string>>
{ { Boss::Msg::Network_Bitcoin
  , { "0231eccc6510eb2e1c97c8a190d6ea096784aa7c358355442055aac8b20654f932@83.162.211.100:9735"
    , "02ad6fb8d693dc1e4569bcedefadf5f72a931ae027dc0f0c544b34c1c6f3b9a02b@167.99.50.31:9735"
    , "03757b80302c8dfe38a127c252700ec3052e5168a7ec6ba183cdab2ac7adad3910@178.128.97.48:11000"
    , "0217890e3aad8d35bc054f43acc00084b25229ecff0ab68debd82883ad65ee8266@23.237.77.11:9735"
    , "03a5886df676f3b3216a4520156157b8d653e262b520281d8d325c24fd8b456b9c@188.165.2.4:9737"
    , "02b686ccf655ece9aec77d4d80f19bb9193f7ce224ab7c8bbe72feb3cdd7187e01@128.199.13.22:9735"
    , "0284a249ee165723f01ae08dc340f9f60bcbce8dda2140401f1bd7548ef11dd5e6@144.76.99.209:9735"
    , "03bb88ccc444534da7b5b64b4f7b15e1eccb18e102db0e400d4b9cfe93763aa26d@138.68.14.104:9735"
    , "033e9ce4e8f0e68f7db49ffb6b9eecc10605f3f3fcb3c630545887749ab515b9c7@46.229.165.150:9735"
    , "03e6ef8c95dbed80bded4d5f36f1754bb401e92b5e6cf55f157f72f5c2b48aa084@189.34.14.93:9735"
    , "03005b000a0ed2b172e7608b062bfe2be18df54769a246941b2cebb5ff2658bb83@170.75.164.184:9735"
    , "03094b63a5ece2c363ab859952017f95bec7bd9e6975f0c8152f9eff83d0b045c2@203.118.186.226:9735"
    , "03456853c17774d597493875deebf8b7b6a781ca7a0d0b83bfaf862070f1ba7322@195.95.224.2:4014"
    , "0318070901e08df311cdc6cdb8a0b4a43a3690c5b32d1fb9d8e99d1a625a65e5f2@3.16.193.211:9735"
    , "02d3db7c13440cf6b4d636208acceb0d07c60e4e14aacd2aa87c0e91ee827422e3@152.32.173.177:9735"
    , "021b5694c643e131b3010bede90677253171210743d971b7d7a9e13c50426863b3@172.81.177.146:9735"
    , "024537120d82069420ff73c1beeb133797a77172f5b8510a8cf9d3f3d8945be42d@173.48.157.250:9735"
    , "03011e480a671ac71dbc3fc3e8fe0db32bb9a10cc4d824663e09557d446ef80679@93.107.153.162:9735"
    , "0298f6074a454a1f5345cb2a7c6f9fce206cd0bf675d177cdbf0ca7508dd28852f@73.119.255.56:9735"
    , "03e30da473716d39c09356d5372cf839d8ee3a1cdbd993ee853c23ab9bdfbce6f8@211.23.128.57:9735"
    , "03e6ef8c95dbed80bded4d5f36f1754bb401e92b5e6cf55f157f72f5c2b48aa084@201.14.128.223:9735"
    , "029a8741675c4c9078b577ddc4348d602d2fb45a12e6087b617925997a84f4c02e@66.68.85.16:9735"
    , "035adf46ca4250dfb8ef43699678b99cde3c1740f6d4723df3ce11ad9c0b0c1829@68.224.6.232:9735"
    , "03f21fc2e8ab0540eeb74dd715b5b66638ec1cd392db00009b320b3ed8c409bd57@35.197.62.113:9735"
    , "03a9b9b4d5bff67fb90d7deaf7db842e3aa5e3abea58fa488a8af3d163679107d7@118.163.74.161:9735"
    }
  }
};

}

namespace Boss { namespace Mod {

class ConnectFinderByHardcode::Impl {
private:
	S::Bus& bus;

	std::vector<std::string> const *nodes;

	void start() {
		bus.subscribe<Msg::Init>([this](Msg::Init const& init) {
			auto it = all_nodes.find(init.network);
			if ( it == all_nodes.end()
			  || it->second.size() == 0
			   )
				return Boss::log( bus, Warn
						, "HardcodedSeeds: "
						  "Cannot seed by hardcoded: "
						  "No known nodes for "
						  "this network."
						);
			nodes = &it->second;
			return Ev::lift();
		});
		bus.subscribe<Msg::SolicitConnectCandidates>([this](Msg::SolicitConnectCandidates const& _) {
			return solicit();
		});
	}

	Ev::Io<void> solicit() {
		if (!nodes)
			return Ev::lift();
		/* Pick 2 nodes.  */
		auto dist = std::uniform_int_distribution<size_t>(
			0, nodes->size() - 1
		);

		auto selected = std::vector<std::string>();
		selected.push_back((*nodes)[dist(random_engine)]);
		selected.push_back((*nodes)[dist(random_engine)]);

		/* Raise it.  */
		return bus.raise(Msg::ProposeConnectCandidates{
			std::move(selected)
		});
	}

public:
	Impl(S::Bus& bus_) : bus(bus_), nodes(nullptr) { start(); }
};

ConnectFinderByHardcode::ConnectFinderByHardcode
		( S::Bus& bus
		) : pimpl(Util::make_unique<Impl>(bus))
		  { }
ConnectFinderByHardcode::ConnectFinderByHardcode
		( ConnectFinderByHardcode&& o
		) : pimpl(std::move(o.pimpl))
		  { }
ConnectFinderByHardcode::~ConnectFinderByHardcode() { }


}}
