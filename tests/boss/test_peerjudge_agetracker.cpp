#undef NDEBUG
#include"Boss/Mod/PeerJudge/AgeTracker.hpp"
#include"Boss/Msg/ChannelCreation.hpp"
#include"Boss/Msg/ChannelDestruction.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include<assert.h>
#include<queue>

namespace {

auto A = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000000");
auto B = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000001");

Ev::Io<void> yieldloop(unsigned int i) {
	return Ev::yield().then([i]() {
		if (i == 0)
			return Ev::lift();
		return yieldloop(i - 1);
	});
}
Ev::Io<void> yield256() {
	return yieldloop(256);
}

auto mock_nows = std::queue<double>();
double mock_get_now() {
	assert(!mock_nows.empty());
	auto rv = mock_nows.front();
	mock_nows.pop();
	return rv;
}

}

int main() {
	S::Bus bus;
	Boss::Mod::PeerJudge::AgeTracker mut(bus, &mock_get_now);

	auto db = Sqlite3::Db(":memory:");

	auto test = Ev::lift().then([&]() {

		/* Send the database.  */
		return bus.raise(Boss::Msg::DbResource{db});
	}).then([&]() {
		return yield256();
	}).then([&]() {

		/* Start with arbitrary node that has not
		 * been seen.
		 * It should get the absolute age.
		 */
		mock_nows.push(1000.0);
		return mut.get_min_age(A);
	}).then([&](double age) {
		assert(age == 1000.0);

		/* Simulate a creation event at time 1000.0,
		 * then query at time 2000.0
		 * We should get an age of 1000.0
		 */
		mock_nows.push(1000.0);
		mock_nows.push(2000.0);
		return bus.raise(Boss::Msg::ChannelCreation{A});
	}).then([&]() {
		return mut.get_min_age(A);
	}).then([&](double age) {
		assert(age == (2000.0 - 1000.0));

		/* Simulate a query at time 3000.0
		 * We should get an age of 2000.0
		 */
		mock_nows.push(3000.0);
		return mut.get_min_age(A);
	}).then([&](double age) {
		assert(age == (3000.0 - 1000.0));

		/* Simulate a query of B at time 4000.0
		 * We should get an age of 4000.0.
		 */
		mock_nows.push(4000.0);
		return mut.get_min_age(B);
	}).then([&](double age) {
		assert(age == 4000.0);

		/* Simulate a destruction event at time 5000.0
		 * then query at time 6000.0.
		 * We should get an age of 1000.0.
		 */
		mock_nows.push(5000.0);
		mock_nows.push(6000.0);
		return bus.raise(Boss::Msg::ChannelDestruction{A});
	}).then([&]() {
		return mut.get_min_age(A);
	}).then([&](double age) {
		assert(age == (6000.0 - 5000.0));

		return Ev::lift(0);
	});

	return 0;
}
