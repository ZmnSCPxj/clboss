#include"Boss/Mod/ChannelCreator/Planner.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include<assert.h>
#include<queue>
#include<random>
#include<set>

namespace Boss { namespace Mod { namespace ChannelCreator {

class Planner::Impl : public std::enable_shared_from_this<Impl> {
private:
	DowserFunc dowser;

	Ln::Amount total;
	Ln::Amount remaining;

	/* Proposals still not being dowsed.  */
	std::queue<std::pair<Ln::NodeId, Ln::NodeId>> proposals;

	/* Whether we should make at least 2 channels.  */
	bool at_least_2;

	/* Limits.  */
	Ln::Amount min_amount;
	Ln::Amount max_amount;

	/* Minimum remaining.  */
	Ln::Amount min_remaining;

	/* Result we are building.  */
	std::map<Ln::NodeId, Ln::Amount> result;
	std::set<Ln::NodeId> rejected;

public:
	Impl( DowserFunc dowser_
	    , Ln::Amount amount_
	    , std::vector<std::pair<Ln::NodeId, Ln::NodeId>> proposals_
	    , std::size_t existing_channels
	    , Ln::Amount min_amount_
	    , Ln::Amount max_amount_
	    , Ln::Amount min_remaining_
	    ) : dowser(std::move(dowser_))
	      , total(amount_), remaining(amount_)
	      , proposals()
	      , at_least_2(existing_channels < 2)
	      , min_amount(min_amount_)
	      , max_amount(max_amount_)
	      , min_remaining(min_remaining_)
	      {
		assert(min_amount * 2 <= total);
		assert(min_amount + min_remaining <= max_amount);
		for (auto& p : proposals_)
			proposals.push(std::move(p));
	}

	Ev::Io<std::map<Ln::NodeId, Ln::Amount>>
	run() {
		auto self = shared_from_this();
		return core_run().then([self]() {
			return Ev::lift(std::move(self->result));
		});
	}

private:
	Ev::Io<void> core_run() {
		/* Loop.  */
		return Ev::yield().then([this]() {
			if (remaining < min_amount)
				return on_complete();
			if (proposals.empty())
				return on_complete();

			return dowser( proposals.front().first
				     , proposals.front().second
				     ).then([this](Ln::Amount amt) {
				auto& node = proposals.front().first;
				if (amt < min_amount) {
					/* Reject.  */
					rejected.insert(node);
					proposals.pop();
					return core_run();
				}
				if (amt > max_amount)
					amt = max_amount;
				if (amt > remaining)
					amt = remaining;
				/* If we need at least 2, and this is
				 * the first proposal, limit it to up
				 * to half of remaining funds.  */
				if ( at_least_2 && result.size() == 0
				  && amt > (remaining / 2)
				   )
					amt = remaining / 2;

				/* Update.  */
				result[node] = amt;
				remaining -= amt;
				proposals.pop();

				return core_run();
			});
		});
	}

	Ev::Io<void> on_complete() {
		return Ev::lift().then([this]() {
			return check_results();
		}).then([this]() {
			return add_rejected();
		});
	}

	Ev::Io<void> check_results() {
		if (at_least_2 && result.size() < 2) {
			/* Not enough proposals.  */
			result.clear();
			return Ev::lift();
		}
		if (remaining > Ln::Amount::sat(0)) {
			/* Still some funds.  */
			/* We never even added anyone?
			 * Probably not enough proposals.  */
			if (result.empty())
				return Ev::lift();

			/* Can we divide it among everyone?  */
			auto remaining_ms = remaining.to_msat();
			auto divided_ms = remaining_ms / result.size();
			auto remainder_ms = remaining_ms % result.size();
			/* Dry run on a copy.  */
			auto can_divide = true;
			auto i = std::size_t(0);
			auto result_copy = result;
			for (auto& r : result_copy) {
				auto divided = Ln::Amount::msat(divided_ms);
				if (i < remainder_ms)
					divided += Ln::Amount::msat(1);
				r.second += divided;
				if (r.second > max_amount) {
					can_divide = false;
					break;
				}
				++i;
			}
			if (can_divide) {
				/* We were able to divide it among everyone.
				 * Promote the dry run.
				 */
				result = std::move(result_copy);
				return Ev::lift();
			}

			if (remaining >= min_remaining)
				/* Just leave it for a rainy day.  */
				return Ev::lift();

			/* If we reached this point, logically it means
			 * at least one of the results is very close to
			 * the max value, meaning it should be far from
			 * the min value.
			 * Just look for an unlucky one we can deduct
			 * from, and either give it to remaining, or
			 * to the next proposal node.
			 */
			auto target = Ln::Amount();
			if (!proposals.empty())
				target = min_amount;
			else
				target = min_remaining;
			/* This is what we need to deduct from one of the
			 * existing results in order to achieve the target.
			 */
			auto needed = target - remaining;
			auto for_deduct = std::vector<Ln::NodeId>();
			for (auto const& r : result)
				if (min_amount + needed <= r.second)
					for_deduct.push_back(r.first);
			assert(!for_deduct.empty());
			auto dist = std::uniform_int_distribution<std::size_t>(
				0, for_deduct.size() - 1
			);
			auto to_deduct = for_deduct[dist(Boss::random_engine)];
			result[to_deduct] -= needed;
			remaining = target;

			if (!proposals.empty()) {
				/* Give the entire remaining to the
				 * next proposal.
				 */
				result[proposals.front().first] = target;
				proposals.pop();
				remaining = Ln::Amount::sat(0);
			}

			return Ev::lift();
		}
		return Ev::lift();
	}

	Ev::Io<void> add_rejected() {
		for (auto& n : rejected)
			result[n] = Ln::Amount::sat(0);
		return Ev::lift();
	}
};

Planner::Planner( DowserFunc dowser
		, Ln::Amount amount
		, std::vector<std::pair<Ln::NodeId, Ln::NodeId>> proposals
		, std::size_t existing_channels
		, Ln::Amount min_amount
		, Ln::Amount max_amount
		, Ln::Amount min_remaining
		) : pimpl(std::make_shared<Impl>( std::move(dowser)
						, amount
						, std::move(proposals)
						, existing_channels
						, min_amount
						, max_amount
						, min_remaining
						))
		  { }

Ev::Io<std::map<Ln::NodeId, Ln::Amount>>
Planner::run()&& {
	return pimpl->run();
}

}}}
