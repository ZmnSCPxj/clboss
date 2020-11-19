#undef NDEBUG
#include"Boss/Mod/OnchainFeeMonitor.hpp"
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include<assert.h>
#include<stdlib.h>

int main() {
	auto db = Sqlite3::Db(":memory:");

	auto bus = S::Bus();
	Boss::Mod::Waiter waiter(bus);
	auto omon = Boss::Mod::OnchainFeeMonitor(bus, waiter);

	auto code = Ev::lift().then([&]() {

		/* Initialize.  */
		auto msg = Boss::Msg::DbResource{db};
		return bus.raise(msg);
	}).then([&]() {

		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		auto samples = std::size_t();
		auto cnt = tx.query(R"QRY(
		SELECT COUNT(*) FROM "OnchainFeeMonitor_samples";
		)QRY")
			.execute();
		for (auto& r : cnt)
			samples = r.get<std::size_t>(0);
		tx.commit();

		assert(samples == 2016);

		/* Check if repetition of DbResource is still 2016.  */
		auto msg = Boss::Msg::DbResource{db};
		return bus.raise(msg);
	}).then([&]() {

		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		auto samples = std::size_t();
		auto cnt = tx.query(R"QRY(
		SELECT COUNT(*) FROM "OnchainFeeMonitor_samples";
		)QRY")
			.execute();
		for (auto& r : cnt)
			samples = r.get<std::size_t>(0);
		tx.commit();

		assert(samples == 2016);

		return Ev::lift(0);
	});

	return Ev::start(code);
}
