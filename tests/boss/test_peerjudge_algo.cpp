#undef NDEBUG
#include"Boss/Mod/PeerJudge/Algo.hpp"
#include"Boss/Mod/PeerJudge/Info.hpp"
#include"Json/Out.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include<assert.h>
#include<cstdint>
#include<iostream>
#include<vector>

namespace {

auto A = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000000");
auto B = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000001");
auto C = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000002");
auto D = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000003");
auto E = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000004");

}

int main() {
	Boss::Mod::PeerJudge::Algo algo;

	/* Initial state should not have any nodes to close.  */
	{
		auto close = algo.get_close();
		assert(close.empty());
	}

	/* Earnings are reasonably close to each other, not large enough
	 * to beat the cost to reopen.  */
	{
		auto inputs = std::vector<Boss::Mod::PeerJudge::Info>
			{ { A, Ln::Amount::sat( 1500000), Ln::Amount::msat( 230000) }
			, { B, Ln::Amount::sat( 2000000), Ln::Amount::msat( 123450) }
			, { C, Ln::Amount::sat( 1000000), Ln::Amount::msat( 399990) }
			, { D, Ln::Amount::sat(25000000), Ln::Amount::msat(5111110) }
			, { E, Ln::Amount::sat(21000000), Ln::Amount::msat(3111110) }
			};
		auto feerate = double(2530.0);
		auto reopen_weight = std::uint64_t(1000);

		algo.judge_peers(inputs, feerate, reopen_weight);

		std::cout << algo.get_status().output() << std::endl;

		auto close = algo.get_close();
		assert(close.empty());
	}

	/* Overfunded peer D, but not worth closing due to high feerate.  */
	{
		auto inputs = std::vector<Boss::Mod::PeerJudge::Info>
			{ { A, Ln::Amount::sat( 1500000), Ln::Amount::msat( 230000) }
			, { B, Ln::Amount::sat( 2000000), Ln::Amount::msat( 123450) }
			, { C, Ln::Amount::sat( 1000000), Ln::Amount::msat( 399990) }
			, { D, Ln::Amount::sat(25000000), Ln::Amount::msat(    100) }
			, { E, Ln::Amount::sat(21000000), Ln::Amount::msat(3111110) }
			};
		auto feerate = double(2530.0);
		auto reopen_weight = std::uint64_t(1000);

		algo.judge_peers(inputs, feerate, reopen_weight);

		std::cout << algo.get_status().output() << std::endl;

		auto close = algo.get_close();
		assert(close.empty());
	}

	/* Overfunded peer D, worth closing at this feerate.  */
	{
		auto inputs = std::vector<Boss::Mod::PeerJudge::Info>
			{ { A, Ln::Amount::sat( 1500000), Ln::Amount::msat( 230000) }
			, { B, Ln::Amount::sat( 2000000), Ln::Amount::msat( 123450) }
			, { C, Ln::Amount::sat( 1000000), Ln::Amount::msat( 399990) }
			, { D, Ln::Amount::sat(25000000), Ln::Amount::msat(    100) }
			, { E, Ln::Amount::sat(21000000), Ln::Amount::msat(3111110) }
			};
		auto feerate = double(253.0);
		auto reopen_weight = std::uint64_t(1000);

		algo.judge_peers(inputs, feerate, reopen_weight);

		std::cout << algo.get_status().output() << std::endl;

		auto close = algo.get_close();
		assert(close.find(D) != close.end());
		assert(close.size() == 1);
		std::cout << close[D] << std::endl;
	}

	return 0;
}
