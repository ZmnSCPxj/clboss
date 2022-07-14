#undef NDEBUG
#include"Boss/Mod/JsonOutputter.hpp"
#include"Boss/Mod/RebalanceBudgeter.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/RequestEarningsInfo.hpp"
#include"Boss/Msg/RequestRebalanceBudget.hpp"
#include"Boss/Msg/ResponseEarningsInfo.hpp"
#include"Boss/Msg/ResponseMoveFunds.hpp"
#include"Boss/Msg/ResponseRebalanceBudget.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include<assert.h>
#include<functional>
#include<iostream>
#include<map>

namespace {

struct EarningsInfo {
	Ln::Amount in_earnings;
	Ln::Amount in_expenditures;
	Ln::Amount out_earnings;
	Ln::Amount out_expenditures;
};

auto const A = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000001");
auto const B = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000002");

Ev::Io<void> repeat( unsigned int i
		   , std::function<Ev::Io<void>()> const& f
		   ) {
	return Ev::yield().then([i, f]() {
		if (i == 0)
			return Ev::lift();
		return f().then([i, f]() {
			return repeat(i - 1, f);
		});
	});
}

}

int main() {
	auto bus = S::Bus();
	auto outputter = Boss::Mod::JsonOutputter(std::cout, bus);

	auto db = Sqlite3::Db(":memory:");

	auto mut = Boss::Mod::RebalanceBudgeter(bus);

	auto get_budget = Boss::ModG::ReqResp< Boss::Msg::RequestRebalanceBudget
					     , Boss::Msg::ResponseRebalanceBudget
					     >(bus);

	/* Earnings information to return.  */
	auto earnings = std::map<Ln::NodeId, EarningsInfo>();
	bus.subscribe<Boss::Msg::RequestEarningsInfo
		     >([ &earnings
		       , &bus
		       ](Boss::Msg::RequestEarningsInfo const& m) {
		auto requester = m.requester;
		auto const& node = m.node;

		auto it = earnings.find(node);
		if (it == earnings.end())
			return bus.raise(Boss::Msg::ResponseEarningsInfo{
				requester, node,
				Ln::Amount::msat(0),
				Ln::Amount::msat(0),
				Ln::Amount::msat(0),
				Ln::Amount::msat(0)
			});

		auto const& ei = it->second;
		return bus.raise(Boss::Msg::ResponseEarningsInfo{
			requester, node,
			ei.in_earnings,
			ei.in_expenditures,
			ei.out_earnings,
			ei.out_expenditures,
		});
	});

	auto code = Ev::lift().then([&]() {

		/* Provide the database.  */
		return bus.raise(Boss::Msg::DbResource{db});
	}).then([&]() {

		/* Check for degenerate case where there
		 * is simply no budget.
		 */
		earnings.clear();
		/* Source has no net earnings.  */
		earnings[A].in_earnings = Ln::Amount::msat(1000);
		earnings[A].in_expenditures = Ln::Amount::msat(1000);
		earnings[A].out_earnings = Ln::Amount::msat(1000);
		earnings[A].out_expenditures = Ln::Amount::msat(0);
		/* Destination has net earnings.  */
		earnings[B].in_earnings = Ln::Amount::msat(50000);
		earnings[B].in_expenditures = Ln::Amount::msat(1000);
		earnings[B].out_earnings = Ln::Amount::msat(1000);
		earnings[B].out_expenditures = Ln::Amount::msat(0);
		/* Get budget.  */
		return get_budget.execute(Boss::Msg::RequestRebalanceBudget{
			nullptr, A, B
		});
	}).then([&](Boss::Msg::ResponseRebalanceBudget r) {
		/* No budget!  */
		assert(r.fee_budget == Ln::Amount::msat(0));

		/* Check for destination-has-no-budget case.  */
		earnings.clear();
		/* Source has net earnings.  */
		earnings[A].in_earnings = Ln::Amount::msat(5000);
		earnings[A].in_expenditures = Ln::Amount::msat(1000);
		earnings[A].out_earnings = Ln::Amount::msat(1000);
		earnings[A].out_expenditures = Ln::Amount::msat(0);
		/* Destination has no net earnings.  */
		earnings[B].in_earnings = Ln::Amount::msat(50000);
		earnings[B].in_expenditures = Ln::Amount::msat(1000);
		earnings[B].out_earnings = Ln::Amount::msat(1000);
		earnings[B].out_expenditures = Ln::Amount::msat(5000);
		/* Get budget.  */
		return get_budget.execute(Boss::Msg::RequestRebalanceBudget{
			nullptr, A, B
		});
	}).then([&](Boss::Msg::ResponseRebalanceBudget r) {
		/* No budget!  */
		assert(r.fee_budget == Ln::Amount::msat(0));

		/* Check for both source and destination having
		 * budgets.
		 */
		earnings.clear();
		/* Source has net earnings.  */
		earnings[A].in_earnings = Ln::Amount::msat(5000);
		earnings[A].in_expenditures = Ln::Amount::msat(1000);
		earnings[A].out_earnings = Ln::Amount::msat(1000);
		earnings[A].out_expenditures = Ln::Amount::msat(0);
		/* Destination has net earnings.  */
		earnings[B].in_earnings = Ln::Amount::msat(50000);
		earnings[B].in_expenditures = Ln::Amount::msat(1000);
		earnings[B].out_earnings = Ln::Amount::msat(500000);
		earnings[B].out_expenditures = Ln::Amount::msat(5000);
		/* Get budget.  */
		return get_budget.execute(Boss::Msg::RequestRebalanceBudget{
			nullptr, A, B
		});
	}).then([&](Boss::Msg::ResponseRebalanceBudget r) {
		/* Should have non-zero budget!  */
		assert(r.fee_budget != Ln::Amount::msat(0));

		/* Now saturate it to tiny budget.
		 * This is done by simulating a lot of rebalance
		 * successes, which implies to the budgeter that
		 * it can afford to lower the budget.
		 */
		return repeat(1000, [&]() {
			return bus.raise(Boss::Msg::ResponseMoveFunds{
				nullptr, Ln::Amount::msat(10),
				Ln::Amount::msat(10)
			});
		});
	}).then([&]() {
		/* Even if there is earnings, if the expenditures
		 * is too near to earnings, it should still have 0
		 * budget.
		 */
		earnings.clear();
		/* Source.  */
		earnings[A].in_earnings = Ln::Amount::msat(10000);
		earnings[A].in_expenditures = Ln::Amount::msat(5000);
		earnings[A].out_earnings = Ln::Amount::msat(10000);
		earnings[A].out_expenditures = Ln::Amount::msat(5000);
		/* Destination.  */
		earnings[B].in_earnings = Ln::Amount::msat(10000);
		earnings[B].in_expenditures = Ln::Amount::msat(5000);
		earnings[B].out_earnings = Ln::Amount::msat(10000);
		earnings[B].out_expenditures = Ln::Amount::msat(5000);
		/* Get budget.  */
		return get_budget.execute(Boss::Msg::RequestRebalanceBudget{
			nullptr, A, B
		});
	}).then([&](Boss::Msg::ResponseRebalanceBudget r) {
		/* Should have zero budget!  */
		assert(r.fee_budget == Ln::Amount::msat(0));

		/* Now saturate it to a high budget.
		 * This is done by simulating a lot of rebalance
		 * failures, which implies to the budgeter that
		 * it should consider increasing the fee budget.
		 * The end result is that the budgeter will now
		 * be willing to give a higher budget.
		 */
		return repeat(1000, [&]() {
			return bus.raise(Boss::Msg::ResponseMoveFunds{
				nullptr, Ln::Amount::msat(0),
				Ln::Amount::msat(0)
			});
		});
	}).then([&]() {
		earnings.clear();
		/* Source.  */
		earnings[A].in_earnings = Ln::Amount::msat(10000);
		earnings[A].in_expenditures = Ln::Amount::msat(5000);
		earnings[A].out_earnings = Ln::Amount::msat(10000);
		earnings[A].out_expenditures = Ln::Amount::msat(5000);
		/* Destination.  */
		earnings[B].in_earnings = Ln::Amount::msat(10000);
		earnings[B].in_expenditures = Ln::Amount::msat(5000);
		earnings[B].out_earnings = Ln::Amount::msat(10000);
		earnings[B].out_expenditures = Ln::Amount::msat(5000);
		/* Get budget.  */
		return get_budget.execute(Boss::Msg::RequestRebalanceBudget{
			nullptr, A, B
		});
	}).then([&](Boss::Msg::ResponseRebalanceBudget r) {
		/* Should have non-zero budget!  */
		assert(r.fee_budget != Ln::Amount::msat(0));

		return Ev::lift(0);
	});

	return Ev::start(code);
}
