#include"Boss/Mod/Tally.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/ForwardFee.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/ManifestCommand.hpp"
#include"Boss/Msg/ResponseMoveFunds.hpp"
#include"Boss/Msg/SwapCompleted.hpp"
#include"Boss/Msg/TimerTwiceDaily.hpp"
#include"Boss/concurrent.hpp"
#include"Ev/Io.hpp"
#include"Ev/now.hpp"
#include"Ev/yield.hpp"
#include"Json/Out.hpp"
#include"Ln/Amount.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Util/date.hpp"
#include"Util/make_unique.hpp"
#include<assert.h>

namespace {

/* How long, in seconds, to keep tally data history.  */
auto constexpr tally_history_age = double(60.0 * 60.0 * 24.0 * 365.0);

}

namespace Boss { namespace Mod {

class Tally::Impl {
private:
	S::Bus& bus;
	Sqlite3::Db db;

	void start() {
		bus.subscribe<Msg::DbResource
			     >([this](Msg::DbResource const& m) {
			db = m.db;
			return init();
		});

		bus.subscribe<Msg::ForwardFee
			     >([this](Msg::ForwardFee const& m) {
			return add_tally( "+forwarding_earnings", m.fee
					, "Total earned from forwarding "
					  "fees."
					);
		});
		bus.subscribe<Msg::ResponseMoveFunds
			     >([this](Msg::ResponseMoveFunds const& m) {
			return add_tally( "-rebalancing_costs", m.fee_spent
					, "Total lost paying for "
					  "rebalances."
					);
		});
		bus.subscribe<Msg::SwapCompleted
			     >([this](Msg::SwapCompleted const& m) {
			/* SwapCompleted is broadcast while another
			 * module has a transaction open.
			 * So do it in the background.
			 */
			auto cost = m.amount_sent - m.amount_received;
			return Boss::concurrent(Ev::lift().then([ this
								, cost
								]() {
				return add_tally( "-inbound_liquidity_swap_costs"
						, cost
						, "Total lost buying inbound "
						  "liquidity using the "
						  "swap-to-onchain technique."
						);
			}));
		});
		/* TODO: opening costs, closing costs, liquidity ads
		 * (earnings from them buying, costs of us buying).  */

		bus.subscribe<Msg::Manifestation
			     >([this](Msg::Manifestation const&) {
			return bus.raise(Msg::ManifestCommand{
				"clboss-cleartally", "",
				"Clear the tally.",
				false
			}) + bus.raise(Msg::ManifestCommand{
				"clboss-tally", "",
				"Get the tally of earnings and "
				"expenditures.",
				false
			});
		});
		bus.subscribe<Msg::CommandRequest
			     >([this](Msg::CommandRequest const& m) {
			if (m.command != "clboss-cleartally")
				return Ev::lift();
			auto id = m.id;
			return db.transact().then([ this
						  , id
						  ](Sqlite3::Tx tx) {
				tx.query_execute(R"QRY(
				DELETE FROM "Tally";
				)QRY");
				tx.query_execute(R"QRY(
				DELETE FROM "Tally_history";
				)QRY");
				tx.commit();
				return bus.raise(Msg::CommandResponse{
					id, Json::Out::empty_object()
				});
			});
		});
		bus.subscribe<Msg::CommandRequest
			     >([this](Msg::CommandRequest const& m) {
			if (m.command != "clboss-tally")
				return Ev::lift();
			auto id = m.id;
			return get_tally().then([ this
						, id
						](Json::Out res) {
				return bus.raise(Msg::CommandResponse{
					id, std::move(res)
				});
			});
		});

		/* Sample the current total and add an entry
		 * to the "Tally_history" table.
		 */
		bus.subscribe<Msg::TimerTwiceDaily
			     >([this](Msg::TimerTwiceDaily const&) {
			return sample_total();
		});
	}

	Ev::Io<void> init() {
		return db.transact().then([](Sqlite3::Tx tx) {
			tx.query_execute(R"QRY(
			CREATE TABLE IF NOT EXISTS "Tally"
			     ( name TEXT PRIMARY KEY
			     , amount INTEGER NOT NULL
			     , comment TEXT NOT NULL
			     );
			CREATE TABLE IF NOT EXISTS "Tally_history"
			     ( time REAL PRIMARY KEY
			     , total INTEGER NOT NULL
			     );
			)QRY");
			tx.commit();
			return Ev::lift();
		});
	}

	Ev::Io<void> add_tally( char const* name
			      , Ln::Amount amount
			      , char const* comment
			      ) {
		assert(name[0] == '+' || name[0] == '-');
		if (!db)
			return Ev::yield().then([ this
						, name, amount, comment
						]() {
				return add_tally(name, amount, comment);
			});
		return db.transact().then([ name, amount, comment
					  ](Sqlite3::Tx tx) {
			auto curr_amount = Ln::Amount::msat(0);
			auto fetch = tx.query(R"SQL(
			SELECT amount FROM "Tally"
			 WHERE name = :name;
			)SQL")
				.bind(":name", name)
				.execute()
				;
			for (auto& r : fetch) {
				curr_amount = Ln::Amount::msat(
					r.get<std::uint64_t>(0)
				);
			}
			tx.query(R"SQL(
			INSERT OR REPLACE INTO "Tally"
			VALUES(:name, :amount, :comment);
			)SQL")
				.bind(":name", name)
				.bind(":amount", (curr_amount + amount).to_msat())
				.bind(":comment", comment)
				.execute()
				;
			tx.commit();

			return Ev::lift();
		});
	}

	Ev::Io<Json::Out> get_tally() {
		if (!db)
			return Ev::yield().then([this]() {
				return get_tally();
			});
		return db.transact().then([](Sqlite3::Tx tx) {
			auto total = std::int64_t(0);

			auto fetch = tx.query(R"QRY(
			SELECT name, amount, comment FROM "Tally";
			)QRY").execute();
			auto result = Json::Out();
			auto obj = result.start_object();
			for (auto& r : fetch) {
				auto name = r.get<std::string>(0);
				auto amount = Ln::Amount::msat(
					r.get<std::uint64_t>(1)
				);
				auto comment = r.get<std::string>(2);
				obj.start_object(name)
					.field("amount", std::string(amount))
					.field("comment", comment)
				.end_object();
				if (name.size() >= 1 && name[0] == '+')
					total += std::int64_t(
						amount.to_msat()
					);
				else
					total -= std::int64_t(
						amount.to_msat()
					);
			}
			obj.field("total", total);
			obj.field( "comment"
				 , "Reset all tallies to 0 "
				   "via `clboss-cleartally`.  "
				   "This data is purely for node "
				   "operator and CLBOSS will never "
				   "use this in its heuristics."
				 );
			auto arr = obj.start_array("history");
			auto fetch2 = tx.query(R"QRY(
			SELECT time, total FROM "Tally_history"
			ORDER BY time ASC;
			)QRY").execute();
			for (auto& r : fetch2) {
				auto time = r.get<double>(0);
				auto total = r.get<std::int64_t>(1);
				arr.start_object()
					.field("time", time)
					.field("time_human", Util::date(time))
					.field("total", total)
				.end_object();
			}
			arr.end_array();
			obj.end_object();

			tx.commit();
			return Ev::lift(std::move(result));
		});
	}

	Ev::Io<void> sample_total() {
		if (!db)
			return Ev::yield().then([this]() {
				return sample_total();
			});
		return db.transact().then([](Sqlite3::Tx tx) {
			auto total = std::int64_t(0);
			auto fetch = tx.query(R"QRY(
			SELECT name, amount FROM "Tally";
			)QRY").execute();
			for (auto& r : fetch) {
				auto name = r.get<std::string>(0);
				auto amount = r.get<std::uint64_t>(1);
				if (name.size() >= 1 && name[0] == '+')
					total += std::int64_t(amount);
				else
					total -= std::int64_t(amount);
			}
			tx.query(R"QRY(
			INSERT OR IGNORE INTO "Tally_history"
			VALUES(:time, :total);
			)QRY")
				.bind(":time", Ev::now())
				.bind(":total", total)
				.execute()
				;
			tx.query(R"QRY(
			DELETE FROM "Tally_history"
			WHERE time < :mintime;
			)QRY")
				.bind( ":mintime"
				     , Ev::now() - tally_history_age
				     )
				.execute()
				;
			tx.commit();

			return Ev::lift();
		});
	}

public:
	Impl() =delete;
	Impl(Impl const&) =delete;
	Impl(Impl&&) =delete;

	explicit
	Impl(S::Bus& bus_) : bus(bus_) { start(); }
};

Tally::Tally(Tally&&) =default;
Tally::~Tally() =default;

Tally::Tally(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
