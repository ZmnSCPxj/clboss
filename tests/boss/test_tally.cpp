#undef NDEBUG
#include"Boss/Mod/Tally.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/ForwardFee.hpp"
#include"Boss/Msg/ResponseMoveFunds.hpp"
#include"Boss/Msg/SwapCompleted.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Util/make_unique.hpp"
#include<assert.h>
#include<cstdint>
#include<iostream>
#include<memory>
#include<sstream>

namespace {

Jsmn::Object to_obj(Json::Out const& o) {
	auto ss = std::istringstream(o.output());
	auto rv = Jsmn::Object();
	ss >> rv;
	return rv;
}

auto const A = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000001");
auto const B = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000002");

}

int main() {
	auto bus = S::Bus();
	auto db = Sqlite3::Db(":memory:");

	auto mut = Boss::Mod::Tally(bus);

	/* Monitor command responses.  */
	auto resp = std::unique_ptr<Boss::Msg::CommandResponse>();
	bus.subscribe< Boss::Msg::CommandResponse
		     >([&](Boss::Msg::CommandResponse const& m) {
		resp = Util::make_unique<Boss::Msg::CommandResponse>(m);
		std::cout << m.response.output() << std::endl;
		return Ev::lift();
	});

	auto code = Ev::lift().then([&]() {
		return bus.raise(Boss::Msg::DbResource{db});
	}).then([&]() {

		/* Get a tally.  */
		resp = nullptr;
		return bus.raise(Boss::Msg::CommandRequest{
			"clboss-tally",
			Jsmn::Object(),
			42
		}) + Ev::yield(42);
	}).then([&]() {
		assert(resp);
		assert(resp->id == 42);
		auto r = to_obj(resp->response);
		assert(r.is_object());
		assert(r.has("total"));
		assert(r["total"].is_number());
		assert(double(r["total"]) == 0.0);

		/* Simulate a couple forwarding events.  */
		return bus.raise(Boss::Msg::ForwardFee{
			A, B, Ln::Amount::msat(42), 0.01
		}) + bus.raise(Boss::Msg::ForwardFee{
			B, A, Ln::Amount::msat(50), 0.01
		}) + Ev::yield(42);
	}).then([&]() {
		/* Get another tally.  */
		resp = nullptr;
		return bus.raise(Boss::Msg::CommandRequest{
			"clboss-tally",
			Jsmn::Object(),
			101
		}) + Ev::yield(42);
	}).then([&]() {
		assert(resp);
		assert(resp->id == 101);
		/* Check response contents.  */
		auto r = to_obj(resp->response);
		assert(r.is_object());
		assert(r.has("+forwarding_earnings"));
		assert(r["+forwarding_earnings"].is_object());
		assert(r["+forwarding_earnings"].has("amount"));
		assert(r["+forwarding_earnings"]["amount"].is_string());
		auto e_s = std::string(r["+forwarding_earnings"]["amount"]);
		assert(Ln::Amount::valid_string(e_s));
		auto e = Ln::Amount(e_s);
		assert(e == (Ln::Amount::msat(42) + Ln::Amount::msat(50)));

		assert(r.has("total"));
		assert(r["total"].is_number());
		assert(std::int64_t(double(r["total"])) == (42 + 50));

		/* Simulate a rebalance.  */
		auto msg = Boss::Msg::ResponseMoveFunds();
		msg.requester = &mut;
		msg.amount_moved = Ln::Amount::msat(500000);
		msg.fee_spent = Ln::Amount::msat(100);
		return bus.raise(msg);
	}).then([&]() {
		/* Get another tally.  */
		resp = nullptr;
		return bus.raise(Boss::Msg::CommandRequest{
			"clboss-tally",
			Jsmn::Object(),
			202
		}) + Ev::yield(42);
	}).then([&]() {
		assert(resp);
		assert(resp->id == 202);
		/* Check response contents.  */
		auto r = to_obj(resp->response);
		assert(r.is_object());
		assert(r.has("-rebalancing_costs"));
		assert(r["-rebalancing_costs"].is_object());
		assert(r["-rebalancing_costs"].has("amount"));
		assert(r["-rebalancing_costs"]["amount"].is_string());
		auto c_s = std::string(r["-rebalancing_costs"]["amount"]);
		assert(Ln::Amount::valid_string(c_s));
		auto c = Ln::Amount(c_s);
		assert(c == Ln::Amount::msat(100));

		assert(r.has("total"));
		assert(r["total"].is_number());
		assert(std::int64_t(double(r["total"])) == (42 + 50 - 100));

		/* Simulate a swap completion.
		 * Need to open a transaction because that is how
		 * a "real" swap would work.
		 */
		return db.transact().then([&](Sqlite3::Tx tx) {
			auto dbtx = std::make_shared<Sqlite3::Tx>(
				std::move(tx)
			);
			return Ev::lift().then([&bus, dbtx]() {
				auto msg = Boss::Msg::SwapCompleted();
				msg.dbtx = dbtx;
				msg.amount_sent = Ln::Amount::msat(500200);
				msg.amount_received = Ln::Amount::msat(500000);
				msg.provider_name = "simulation";
				return bus.raise(msg);
			}).then([dbtx]() {
				dbtx->commit();
				return Ev::yield(42);
			});
		});
	}).then([&]() {
		/* Get another tally.  */
		resp = nullptr;
		return bus.raise(Boss::Msg::CommandRequest{
			"clboss-tally",
			Jsmn::Object(),
			303
		}) + Ev::yield(42);
	}).then([&]() {
		assert(resp);
		assert(resp->id == 303);
		/* Check response contents.  */
		auto r = to_obj(resp->response);
		assert(r.is_object());
		assert(r.has("-inbound_liquidity_swap_costs"));
		assert(r["-inbound_liquidity_swap_costs"].is_object());
		assert(r["-inbound_liquidity_swap_costs"].has("amount"));
		assert(r["-inbound_liquidity_swap_costs"]["amount"].is_string());
		auto c_s = std::string(r["-inbound_liquidity_swap_costs"]["amount"]);
		assert(Ln::Amount::valid_string(c_s));
		auto c = Ln::Amount(c_s);
		assert(c == Ln::Amount::msat(200));

		assert(r.has("total"));
		assert(r["total"].is_number());
		assert(std::int64_t(double(r["total"])) == (42 + 50 - 100 - 200));

		/* Clear the tally.  */
		resp = nullptr;
		return bus.raise(Boss::Msg::CommandRequest{
			"clboss-cleartally",
			Jsmn::Object(),
			404
		}) + Ev::yield(42);
	}).then([&]() {
		assert(resp);
		assert(resp->id == 404);

		/* Get another tally.  */
		resp = nullptr;
		return bus.raise(Boss::Msg::CommandRequest{
			"clboss-tally",
			Jsmn::Object(),
			505
		}) + Ev::yield(42);
	}).then([&]() {
		assert(resp);
		assert(resp->id == 505);
		/* Check response contents.  */
		auto r = to_obj(resp->response);
		assert(r.is_object());
		assert(r.has("total"));
		assert(r["total"].is_number());
		assert(std::int64_t(double(r["total"])) == 0);

		return Ev::lift(0);
	});

	return Ev::start(code);
}
