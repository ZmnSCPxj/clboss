#undef NDEBUG
#include"Boss/Mod/PeerComplaintsDesk/Recorder.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"
#include"Ln/NodeId.hpp"
#include"Sqlite3.hpp"
#include<assert.h>

namespace {

auto A = Ln::NodeId("02000000000000000000000000000000000000000000000000000000000000000A");
auto B = Ln::NodeId("02000000000000000000000000000000000000000000000000000000000000000B");
auto C = Ln::NodeId("02000000000000000000000000000000000000000000000000000000000000000C");

}

int main() {
	using namespace Boss::Mod::PeerComplaintsDesk::Recorder;

	auto db = Sqlite3::Db(":memory:");

	auto code = Ev::lift().then([&]() {

		/* Repeated initialize should be fine.  */
		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		initialize(tx);
		initialize(tx);
		tx.commit();
		/* Even in different transactions.  */
		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		initialize(tx);
		tx.commit();

		/* Add some complaints.  */
		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		add_complaint(tx, A, "complaint A 1");
		add_complaint(tx, A, "complaint A 2");
		add_complaint(tx, B, "complaint B 1");
		add_ignored_complaint(tx, C, "complaint C 1", "C is awesome");
		tx.commit();

		/* Check complaints.  */
		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		auto record = check_complaints(tx);
		assert(record[A] == 2);
		assert(record[B] == 1);
		/* The complaint on C is ignored, so not counted and not
		 * included.  */
		assert(record.find(C) == record.end());
		tx.commit();

		/* Cleanup of really really old complaints should not
		 * affect above, which are really new.  */
		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		cleanup(tx, 30 * 24 * 3600, 60 * 24 * 3600);
		/* Just a repeat of the above.  */
		auto record = check_complaints(tx);
		assert(record[A] == 2);
		assert(record[B] == 1);
		assert(record.find(C) == record.end());
		tx.commit();

		/* Gather complaints into human-readable strings.  */
		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		auto record = get_all_complaints(tx);
		assert(record[A].size() == 2);
		assert(record[B].size() == 1);
		/* Includes ignored ones.  */
		assert(record[C].size() == 1);
		/* Check no closed complaints yet.  */
		auto closed = get_closed_complaints(tx);
		assert(closed.empty());
		tx.commit();

		/* Add more complaints to C.  */
		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		add_complaint(tx, C, "complaint C 2");
		add_complaint(tx, C, "complaint C 3");
		tx.commit();

		/* Close C.  */
		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		channel_closed(tx, C);
		auto record = get_all_complaints(tx);
		assert(record.find(C) == record.end());
		auto closed = get_closed_complaints(tx);
		assert(closed[C].size() == 2);
		tx.commit();

		/* Pointlessly delay a few times.
		 * This is only to let libev advance its sense of
		 * time in Ev::now().
		 */
		return Ev::yield() + Ev::yield() + Ev::yield() + Ev::yield();
	}).then([&]() {
		/* Cleaning up all older than 0 seconds ago should
		 * clear all the complaints we added before.  */
		return db.transact();
	}).then([&](Sqlite3::Tx tx) {
		cleanup(tx, 0, 0);
		auto record = get_all_complaints(tx);
		assert(record.empty());
		auto closed = get_closed_complaints(tx);
		assert(closed.empty());
		tx.commit();

		return Ev::lift();
	}).then([]() {
		return Ev::lift(0);
	});

	return Ev::start(code);
}

