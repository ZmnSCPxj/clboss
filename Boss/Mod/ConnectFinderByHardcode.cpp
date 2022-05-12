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
  , { "0231eccc6510eb2e1c97c8a190d6ea096784aa7c358355442055aac8b20654f932@77.167.245.239:9735"
    , "02ad6fb8d693dc1e4569bcedefadf5f72a931ae027dc0f0c544b34c1c6f3b9a02b@167.99.50.31:9735"
    , "0217890e3aad8d35bc054f43acc00084b25229ecff0ab68debd82883ad65ee8266@23.237.77.11:9735"
    , "03a5886df676f3b3216a4520156157b8d653e262b520281d8d325c24fd8b456b9c@188.165.2.4:9737"
    , "02b686ccf655ece9aec77d4d80f19bb9193f7ce224ab7c8bbe72feb3cdd7187e01@128.199.13.22:9735"
    , "0284a249ee165723f01ae08dc340f9f60bcbce8dda2140401f1bd7548ef11dd5e6@144.76.99.209:9735"
    , "03bb88ccc444534da7b5b64b4f7b15e1eccb18e102db0e400d4b9cfe93763aa26d@138.68.14.104:9735"
    , "033e9ce4e8f0e68f7db49ffb6b9eecc10605f3f3fcb3c630545887749ab515b9c7@46.229.165.150:9735"
    , "03005b000a0ed2b172e7608b062bfe2be18df54769a246941b2cebb5ff2658bb83@170.75.164.184:9735"
    , "03094b63a5ece2c363ab859952017f95bec7bd9e6975f0c8152f9eff83d0b045c2@203.118.186.226:9735"
    , "03456853c17774d597493875deebf8b7b6a781ca7a0d0b83bfaf862070f1ba7322@195.95.224.2:4014"
    , "02d3db7c13440cf6b4d636208acceb0d07c60e4e14aacd2aa87c0e91ee827422e3@152.32.173.177:9735"
    , "0298f6074a454a1f5345cb2a7c6f9fce206cd0bf675d177cdbf0ca7508dd28852f@73.114.40.246:9735"
    , "03e30da473716d39c09356d5372cf839d8ee3a1cdbd993ee853c23ab9bdfbce6f8@211.23.128.57:9735"
    , "03f21fc2e8ab0540eeb74dd715b5b66638ec1cd392db00009b320b3ed8c409bd57@35.197.62.113:9735"
    , "03a9b9b4d5bff67fb90d7deaf7db842e3aa5e3abea58fa488a8af3d163679107d7@118.163.74.161:9735"
    , "033868c219bdb51a33560d854d500fe7d3898a1ad9e05dd89d0007e11313588500@52.204.16.4:9735"
    , "028919036e106a4d58d35d2ca59dc05159e57baee651f9634e0639f00613a4cd3f@3.138.90.63:9735"
    , "02c911ba09b21441200e0424279720ec13290b36cd51f4ab1a80cbb40b3baf0fe1@202.185.146.109:9735"
    , "032434517e28f7b51665a525d5e11fa493bd4e30a59883b8156c2be4085f4aaf70@46.6.9.152:9735"
    , "029dd81c32e3ea839b4b2f8edcc9bea227c40fa2c6509878034401a8ce2a8bac05@176.36.37.21:9735"
    , "0215e9649c6a14ca64109418a5b17b0ddefd446e8256486a0c692e1fd8ec72e316@200.75.4.254:9735"
    , "03c5528c628681aa17ab9e117aa3ee6f06c750dfb17df758ecabcd68f1567ad8c1@82.16.240.248:9735"
    , "035fed4182fbd0725264f8a0018cabb6b25514dd231291162ac8dd63afb278e9e8@68.183.121.169:9735"
    , "03b323009613777edc4e8024c6726758396bc026da99545971164a6baad31dfc69@18.176.200.244:9735"
    , "028c8c9b0b588d00afabe905799aee0225a9315cdb63d7646b7ff7cf02fe4bf643@197.155.6.43:9735"
    , "0326f1d06748f019ecd98484eb23ba7367b1361e0ee6e38234d73afcc976dfdb3d@190.85.201.37:9735"
    , "03f3f3f9ea1e6e3df8108a3f11031f0fba8e494535c5a45fb83bdc95a89fbe18b7@189.212.121.74:9735"
    , "035e3aa81503b67d61589270c043bcf6dcf083e40bc8630eeeeb08f85570be0fc9@185.123.207.109:9735"
    , "02309f3556d51e9d855594762dcf68cb283fbfd7e48618b6cecb8cd90e2e34d8ad@79.62.233.36:9735"
    , "0321e0fedfda0722c3eaa866cea8a23540dae4dfbaf247785f7fe111b0ea7a53ec@49.0.65.207:9735"
    , "027e70d8859e090ed975b11cfa02109221482954a254dff185e4142188eb66518b@47.75.125.192:9735"
    , "0237fefbe8626bf888de0cad8c73630e32746a22a2c4faa91c1d9877a3826e1174@13.48.89.186:9735"
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
