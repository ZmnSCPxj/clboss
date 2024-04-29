#undef NDEBUG
#include"Boss/Mod/PeerJudge/DataGatherer.hpp"
#include"Boss/Mod/PeerJudge/Info.hpp"
#include"Boss/Msg/ListpeersResult.hpp"
#include"Boss/Msg/OnchainFee.hpp"
#include"Boss/Msg/PeerStatistics.hpp"
#include"Boss/Msg/RequestPeerStatistics.hpp"
#include"Boss/Msg/ResponsePeerStatistics.hpp"
#include"Ev/now.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<assert.h>
#include<map>
#include<memory>
#include<sstream>
#include<stdlib.h>
#include<vector>

namespace {

Ev::Io<void> yieldloop(size_t count) {
	return Ev::yield().then([count]() {
		if (count == 0)
			return Ev::lift();
		return yieldloop(count - 1);
	});
}

Ev::Io<void> yield256() {
	return yieldloop(256);
}

auto timeframe = double(2016 * 10 * 60);

}

int main() {
	using Boss::Mod::PeerJudge::Info;
	using Boss::Msg::PeerStatistics;

	S::Bus bus;

	/* Receiver of information.  */
	auto received = false;
	auto infos = std::vector<Info>();
	auto feerate = double(0);
	auto info_receiver = [&]( std::vector<Info> new_info
				, double new_feerate
				) {
		received = true;
		infos = std::move(new_info);
		feerate = new_feerate;
		return Ev::lift();
	};
	auto clear_received = [&]() {
		received = false;
		infos.clear();
	};

	/* Age model.  */
	auto ages = std::map<Ln::NodeId, double>();
	auto get_min_age = [&](Ln::NodeId id) {
		auto it = ages.find(id);
		if (it == ages.end())
			return Ev::lift(999999999.0);
		return Ev::lift(it->second);
	};

	/* PeerStatistics model.  */
	auto stats = std::map<Ln::NodeId, PeerStatistics>();
	bus.subscribe< Boss::Msg::RequestPeerStatistics
		     >([&](Boss::Msg::RequestPeerStatistics const& m) {
		auto requester = m.requester;
		auto timediff = m.end_time - m.start_time;
		assert(timeframe * 0.999 <= timediff && timediff <= timeframe * 1.001);

		return bus.raise(Boss::Msg::ResponsePeerStatistics{
			requester, stats
		});
	});

	/* ListpeersResult model.  */
	auto listpeers = [&](std::string peers_json) {
		std::cerr << "LISTPEERS: " << peers_json << std::endl;
		auto is = std::stringstream(std::move(peers_json));
		auto peers = Jsmn::Object();
		is >> peers;
		return bus.raise(Boss::Msg::ListpeersResult{
				Boss::Mod::convert_legacy_listpeers(peers), false
		}).then([]() {
			/* Let object run.  */
			return yield256();
		});
	};
	/* OnchainFee model.  */
	auto setfeerate = [&](double feerate_perkw) {
		return bus.raise(Boss::Msg::OnchainFee{
			true,
			Util::make_unique<double>(feerate_perkw)
		});
	};
	(void) setfeerate;

	/* Module under test.  */
	Boss::Mod::PeerJudge::DataGatherer mut( bus
					      , timeframe
					      , get_min_age
					      , info_receiver
					      );

	/* Test code.  */
	auto test = Ev::lift().then([&]() {

		/* Dummy.  */
		return listpeers(R"JSON(
			[]
		)JSON");
	}).then([&]() {
		assert(!received);
		clear_received();

		/* Even with data, without the feerate being sent first,
		 * there should still not be any data gathered.
		 */
		return listpeers(R"JSON(
			[ { "id": "020000000000000000000000000000000000000000000000000000000000000000"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1000000000msat"
					  }
					]
			  }
			]
		)JSON");
	}).then([&]() {
		assert(!received);
		clear_received();

		/* Now send feerate
		 * Even with feerate, wtihout data from statistics,
		 * there should still not be any data gathered.
		 */
		return setfeerate(253.0);
	}).then([&]() {
		return listpeers(R"JSON(
			[ { "id": "020000000000000000000000000000000000000000000000000000000000000000"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1000000000msat"
					  }
					]
			  }
			]
		)JSON");
	}).then([&]() {
		assert(!received);
		clear_received();

		/* Now set statistics.  * */
		auto id = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000000");
		auto& stat = stats[id];
		stat.start_time = Ev::now() - timeframe;
		stat.end_time = Ev::now();
		stat.age = timeframe + 1;
		stat.in_fee = Ln::Amount::sat(100);
		stat.out_fee = Ln::Amount::sat(100);
		return listpeers(R"JSON(
			[ { "id": "020000000000000000000000000000000000000000000000000000000000000000"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1000000000msat"
					  }
					]
			  }
			]
		)JSON");
	}).then([&]() {
		assert(received);
		assert(infos.size() == 1);
		assert(infos[0].id == Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000000"));
		assert(infos[0].total_normal == Ln::Amount::msat(1000000000));
		assert(infos[0].earned == (Ln::Amount::sat(100) + Ln::Amount::sat(100)));
		clear_received();

		/* Now include another peer, but without statistics.  */
		return listpeers(R"JSON(
			[ { "id": "020000000000000000000000000000000000000000000000000000000000000000"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1000000000msat"
					  }
					]
			  }
			, { "id": "020000000000000000000000000000000000000000000000000000000000000001"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1500000000msat"
					  }
					]
			  }
			]
		)JSON");
	}).then([&]() {
		assert(received);
		assert(infos.size() == 1);
		clear_received();

		/* Now add statistics for the second peer but make it too young.  */
		auto id = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000001");
		auto& stat = stats[id];
		stat.start_time = Ev::now() - timeframe;
		stat.end_time = Ev::now();
		stat.age = timeframe / 2.0;
		stat.in_fee = Ln::Amount::sat(150);
		stat.out_fee = Ln::Amount::sat(150);
		return listpeers(R"JSON(
			[ { "id": "020000000000000000000000000000000000000000000000000000000000000000"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1000000000msat"
					  }
					]
			  }
			, { "id": "020000000000000000000000000000000000000000000000000000000000000001"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1500000000msat"
					  }
					]
			  }
			]
		)JSON");
	}).then([&]() {
		assert(received);
		assert(infos.size() == 1);
		clear_received();

		/* Now give it a proper age.  */
		auto id = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000001");
		auto& stat = stats[id];
		stat.age = timeframe + 1;
		return listpeers(R"JSON(
			[ { "id": "020000000000000000000000000000000000000000000000000000000000000000"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1000000000msat"
					  }
					]
			  }
			, { "id": "020000000000000000000000000000000000000000000000000000000000000001"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1500000000msat"
					  }
					]
			  }
			]
		)JSON");
	}).then([&]() {
		assert(received);
		assert(infos.size() == 2);
		assert(infos[1].id == Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000001"));
		clear_received();

		/* Make first peer too young via get_min_age.  */
		auto id = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000000");
		ages[id] = timeframe / 2.0;
		return listpeers(R"JSON(
			[ { "id": "020000000000000000000000000000000000000000000000000000000000000000"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1000000000msat"
					  }
					]
			  }
			, { "id": "020000000000000000000000000000000000000000000000000000000000000001"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1500000000msat"
					  }
					]
			  }
			]
		)JSON");
	}).then([&]() {
		assert(received);
		assert(infos.size() == 1);
		assert(infos[0].id == Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000001"));
		clear_received();

		/* Malformed result from listpeers.  */
		return listpeers(R"JSON(
			[ { "mogumogu": "020000000000000000000000000000000000000000000000000000000000000000"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1000000000msat"
					  }
					]
			  }
			, { "id": "020000000000000000000000000000000000000000000000000000000000000001"
			  , "channels": [ { "state": "CHANNELD_NORMAL"
					  , "total_msat": "1500000000msat"
					  }
					]
			  }
			, { "id": "020000000000000000000000000000000000000000000000000000000000000002"
			  , "channels": [ { "stateXX": "CHANNELD_NORMAL"
					  , "total_msat": "1500000000msat"
					  }
					]
			  }
			]
		)JSON");
	}).then([&]() {
		assert(!received);
		clear_received();

		return Ev::lift(0);
	});

	return Ev::start(test);
}
