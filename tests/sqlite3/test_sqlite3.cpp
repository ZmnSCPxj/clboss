#undef NDEBUG
#include"Sqlite3.hpp"
#include"Ev/Io.hpp"
#include"Ev/concurrent.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"
#include<assert.h>

#include<iostream>

int main() {
	auto db = Sqlite3::Db(":memory:");

	auto empty_transaction = [&]() {
		return db.transact().then([&](Sqlite3::Tx tx) {
			tx.commit();
			return Ev::lift();
		});
	};

	auto code = Ev::lift().then([&]() {

		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		assert(tx);
		tx.commit();
		assert(!tx);

		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		assert(tx);
		tx.rollback();
		assert(!tx);

		/* Test concurrency.  */
		return Ev::concurrent(empty_transaction());
	}).then([&]() {
		return Ev::concurrent(empty_transaction());
	}).then([&]() {
		return Ev::concurrent(empty_transaction());
	}).then([&]() {
		return Ev::yield();
	}).then([&]() {

		/* Test simple interface.  */
		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		tx.query_execute("CREATE TABLE \"foo\" (c1 INTEGER, c2 TEXT);");
		tx.commit();

		/* Test full query interface.  */
		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		auto res = tx.query("INSERT INTO \"foo\" VALUES(:c1, :c2)")
			.bind(":c1", 42)
			.bind(":c2", "some text")
			.execute()
			;
		for (auto& r : res) {
			(void) r;
			/* Should have empty result!  */
			assert(false);
		}
		tx.commit();

		/* Test full query interface again.  */
		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		auto res = tx.query("SELECT c1, c2 FROM \"foo\"")
			.execute()
			;
		auto flag = false;
		for (auto& r : res) {
			/* Should have single result!  */
			assert(!flag);
			flag = true;
			/* Should be what we inserted.  */
			assert(r.get<int>(0) == 42);
			assert(r.get<std::string>(1) == "some text");
		}
		/* Should have result!  */
		assert(flag);
		tx.commit();

		return Ev::lift(0);
	});

	return Ev::start(code);
}
