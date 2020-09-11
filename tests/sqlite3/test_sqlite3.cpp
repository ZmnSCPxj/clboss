#undef NDEBUG
#include"Sqlite3/Db.hpp"
#include"Sqlite3/Tx.hpp"
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

		return Ev::lift(0);
	});

	return Ev::start(code);
}
