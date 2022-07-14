#include"Boss/Mod/RebalanceBudgeter.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/RequestEarningsInfo.hpp"
#include"Boss/Msg/RequestRebalanceBudget.hpp"
#include"Boss/Msg/ResponseEarningsInfo.hpp"
#include"Boss/Msg/ResponseMoveFunds.hpp"
#include"Boss/Msg/ResponseRebalanceBudget.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
#include"Ev/map.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"
#include<cstdint>
#include<math.h>
#include<random>
#include<vector>

namespace {

/* Range of allocated percentage.  */
auto constexpr min_budget = std::uint64_t(1);
auto constexpr max_budget = std::uint64_t(85);
/* Starting percentage.  */
auto constexpr start_budget = std::uint64_t(50);

}

namespace Boss { namespace Mod {

class RebalanceBudgeter::Impl {
private:
	S::Bus& bus;

	Sqlite3::Db db;
	std::uint8_t budget;
	ModG::ReqResp< Msg::RequestEarningsInfo
		     , Msg::ResponseEarningsInfo
		     > get_earnings;

	std::uniform_int_distribution<std::uint64_t> dist_budget;

	void start() {
		bus.subscribe<Msg::DbResource
			     >([this](Msg::DbResource const& m) {
			db = m.db;
			return db.transact().then([this](Sqlite3::Tx tx) {
				/* We store a REAL number into the database
				 * in case in the future we decide to change
				 * completely how this module changes the
				 * budget allocation.
				 * Currently we just do a 1% movement of the
				 * budget allocation.
				 */
				tx.query_execute(R"QRY(
				CREATE TABLE IF NOT EXISTS "RebalanceBudgeter"
				     ( id INTEGER PRIMARY KEY
				     , budget REAL NOT NULL
				     );
				)QRY");
				tx.query(R"QRY(
				INSERT OR IGNORE INTO "RebalanceBudgeter"
				VALUES(0, :budget);
				)QRY")
					.bind( ":budget"
					     , double(start_budget) / 100.0
					     )
					.execute()
					;
				auto fetch = tx.query(R"QRY(
				SELECT budget FROM "RebalanceBudgeter";
				)QRY").execute();
				for (auto& r : fetch) {
					budget = std::uint8_t(round(
						r.get<double>(0) * 100.0
					));
					break;
				}
				tx.commit();

				return Ev::lift();
			});
		});

		bus.subscribe<Msg::ResponseMoveFunds
			     >([this](Msg::ResponseMoveFunds const& m) {
			if (m.amount_moved == Ln::Amount::msat(0))
				return maybe_change_budget( +1
							  , "FundsMover "
							    "failed to "
							    "rebalance"
							  );
			else
				return maybe_change_budget( -1
							  , "FundsMover "
							    "rebalanced "
							    "successfully"
							  );
		});

		bus.subscribe<Msg::RequestRebalanceBudget
			     >([this](Msg::RequestRebalanceBudget const& m) {
			auto requester = m.requester;
			auto budget_d = double(budget) / 100.0;
			auto const& source = m.source;
			auto const& destination = m.destination;
			auto nodes = std::vector<Ln::NodeId>{
				source, destination
			};
			auto f = [this](Ln::NodeId const& n) {
				return get_earnings.execute(Msg::RequestEarningsInfo{
					nullptr, n
				});
			};
			return Ev::map( std::move(f)
				      , std::move(nodes)
				      ).then([ this
					     , requester
					     , budget_d
					     , source
					     , destination
				             ](std::vector<Msg::ResponseEarningsInfo> earnings) {
				auto s_earn = earnings[0].in_earnings;
				auto s_lim = s_earn * budget_d;
				auto s_spent = earnings[0].in_expenditures;
				auto d_earn = earnings[1].out_earnings;
				auto d_lim = d_earn * budget_d;
				auto d_spent = earnings[1].out_expenditures;

				/* Out of budget conditions.  */
				if (s_spent >= s_lim)
					return Boss::log( bus, Info
							, "RebalanceBudgeter: "
							  "Source %s "
							  "already spent %s "
							  "on rebalances "
							  "(limit is %s)"
							, Util::stringify(source)
								.c_str()
							, Util::stringify(s_spent)
								.c_str()
							, Util::stringify(s_lim)
								.c_str()
							)
					     + bus.raise(Msg::ResponseRebalanceBudget{
						requester,
						Ln::Amount::msat(0)
					});
				if (d_spent >= d_lim)
					return Boss::log( bus, Info
							, "RebalanceBudgeter: "
							  "Destination %s "
							  "already spent %s "
							  "on rebalances "
							  "(limit is %s)"
							, Util::stringify(destination)
								.c_str()
							, Util::stringify(d_spent)
								.c_str()
							, Util::stringify(d_lim)
								.c_str()
							)
					     + bus.raise(Msg::ResponseRebalanceBudget{
						requester,
						Ln::Amount::msat(0)
					});

				/* *Remaining* budget is the budget limit
				 * minus the already-spent budget.  */
				auto s_budget = s_lim - s_spent;
				auto d_budget = d_lim - d_spent;
				/* The final budget is the lower of the
				 * source or destination budgets.  */
				auto f_budget =
					(s_budget > d_budget) ? d_budget : s_budget;
				return bus.raise(Msg::ResponseRebalanceBudget{
					requester,
					f_budget
				});
			});
		});
	}

	Ev::Io<void> maybe_change_budget( std::int8_t dir
					, char const* reason
					) {
		return Ev::lift().then([this, dir, reason]() {
			auto num = dist_budget(Boss::random_engine);
			if (dir < 0) {
				if (num >= budget)
					return Ev::lift();
			} else {
				if (num <= budget)
					return Ev::lift();
			}
			/* We should update.  */
			return db.transact().then([ this
						  , dir
						  , reason
						  ](Sqlite3::Tx tx) {
				auto old_budget = budget;
				if (dir < 0)
					--budget;
				else
					++budget;

				tx.query(R"QRY(
				UPDATE "RebalanceBudgeter"
				   SET budget = :budget;
				)QRY")
					.bind( ":budget"
					     , double(budget) / 100.0
					     )
					.execute()
					;
				tx.commit();

				return Boss::log( bus, Info
						, "RebalanceBudgeter: %s: "
						  "changed budget: "
						  "%u%% -> %u%%"
						, reason
						, (unsigned int) old_budget
						, (unsigned int) budget
						);
			});
		});
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;

	explicit
	Impl( S::Bus& bus_
	    ) : bus(bus_)
	      , get_earnings(bus_)
	      , dist_budget(min_budget, max_budget)
	      { start(); }
};

/* Use default implementations.  */
RebalanceBudgeter::RebalanceBudgeter(RebalanceBudgeter&&) =default;
RebalanceBudgeter::~RebalanceBudgeter() =default;

/* Construct the implementation.  */
RebalanceBudgeter::RebalanceBudgeter(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
