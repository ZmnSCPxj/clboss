#undef NDEBUG
#include"Boss/Mod/ChannelCreator/Planner.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include<algorithm>
#include<assert.h>
#include<iterator>
#include<map>

namespace {

auto A = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000000");
auto B = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000001");
auto C = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000002");
auto D = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000003");

auto min_amount = Ln::Amount::btc(0.005);
auto max_amount = Ln::Amount::btc(0.160);
auto min_remaining = Ln::Amount::btc(0.011);

}

int main() {
	auto dowser_map = std::map<Ln::NodeId, Ln::Amount>();
	auto dowser = [&dowser_map](Ln::NodeId n, Ln::NodeId ignore) {
		return Ev::lift(dowser_map[n]);
	};

	auto planner = [&dowser]( Ln::Amount amount
				, std::vector<Ln::NodeId> proposals
				, std::size_t existing_channels
				) {
		auto n_proposals = std::vector<std::pair<Ln::NodeId, Ln::NodeId>>();
		std::transform( proposals.begin(), proposals.end()
			      , std::back_inserter(n_proposals)
			      , [](Ln::NodeId n) {
			return std::make_pair(std::move(n), Ln::NodeId());
		});

		auto obj = Boss::Mod::ChannelCreator::Planner
			( dowser
			, amount
			, std::move(n_proposals)
			, existing_channels
			, min_amount
			, max_amount
			, min_remaining
			);
		return std::move(obj).run();
	};

	/* Computes the remaining amount from the given original
	 * amount minus all the planned amounts.
	 */
	auto validate_remaining = []( Ln::Amount amount
				    , std::map< Ln::NodeId, Ln::Amount
					      > const& plan
				    ) {
		for (auto const& p : plan)
			amount -= p.second;
		/* Either there is no remainder, or it is above
		 * min_remaining.  */
		assert( amount == Ln::Amount()
		     || amount >= min_remaining
		      );
		return amount;
	};

	auto code = Ev::lift().then([&]() {

		/* Try with A having too much liquidity.  */
		dowser_map.clear();
		dowser_map[A] = Ln::Amount::btc(1.0);
		dowser_map[B] = Ln::Amount::btc(1.0);
		dowser_map[C] = Ln::Amount::btc(1.0);
		return planner( Ln::Amount::btc(0.163)
			      , {A, B, C}
			      , 4
			      );
	}).then([&](std::map<Ln::NodeId, Ln::Amount> result) {
		assert(result.find(A) != result.end());
		assert(result.find(B) != result.end());
		assert(result[B] == min_amount);
		validate_remaining(Ln::Amount::btc(0.163), result);

		/* Try with everyone having low liquidity.  */
		dowser_map.clear();
		dowser_map[A] = Ln::Amount::btc(0.002);
		dowser_map[B] = Ln::Amount::btc(0.002);
		dowser_map[C] = Ln::Amount::btc(0.002);
		return planner( Ln::Amount::btc(0.011)
			      , {A, B, C}
			      , 0
			      );
	}).then([&](std::map<Ln::NodeId, Ln::Amount> result) {
		/* All of them should have been rejected, with 0 assigned
		 * capacity.  */
		assert(result.find(A) != result.end());
		assert(result.find(B) != result.end());
		assert(result.find(C) != result.end());
		assert(result[A] == Ln::Amount::sat(0));
		assert(result[B] == Ln::Amount::sat(0));
		assert(result[C] == Ln::Amount::sat(0));
		auto remaining = validate_remaining( Ln::Amount::btc(0.011)
						   , result
						   );
		assert(remaining == Ln::Amount::btc(0.011));

		/* Not a lot of proposals but we have lots of funds.  */
		dowser_map.clear();
		dowser_map[A] = Ln::Amount::btc(1.0);
		return planner( Ln::Amount::btc(0.163)
			      , {A}
			      , 4
			      );
	}).then([&](std::map<Ln::NodeId, Ln::Amount> result) {
		/* We already have existing channels, so we would go with
		 * just one channel.  */
		assert(result.size() == 1);
		assert(result.find(A) != result.end());
		assert(result[A] <= max_amount);
		auto remaining = validate_remaining(Ln::Amount::btc(0.163), result);
		/* There should be some remainder.  */
		assert(remaining != Ln::Amount::sat(0));

		return Ev::lift(0);
	});
	return Ev::start(code);
}
