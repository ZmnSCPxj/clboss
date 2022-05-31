#undef NDEBUG
#include"Boss/Mod/PeerStatistician.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/InternetOnline.hpp"
#include"Boss/Msg/ListpeersAnalyzedResult.hpp"
#include"Boss/Msg/RequestPeerStatistics.hpp"
#include"Boss/Msg/ResponsePeerStatistics.hpp"
#include"Ev/Io.hpp"
#include"Ev/now.hpp"
#include"Ev/start.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include<assert.h>

namespace {

auto const A = Ln::NodeId("020000000000000000000000000000000000000000000000000000000000000000");

}

int main() {
	auto db = Sqlite3::Db(":memory:");
	auto bus = S::Bus();
	auto statistician = Boss::Mod::PeerStatistician(bus);

	/* Interface to the statistician.  */
	auto reqresp = Boss::ModG::ReqResp< Boss::Msg::RequestPeerStatistics
					  , Boss::Msg::ResponsePeerStatistics
					  >
			(bus);
	auto get_stats = [&]() {
		using Boss::Msg::RequestPeerStatistics;
		using Boss::Msg::ResponsePeerStatistics;
		return Ev::lift().then([&]() {
			return reqresp.execute(RequestPeerStatistics{
				nullptr, Ev::now() - 100, Ev::now()
			});
		}).then([&](ResponsePeerStatistics m) {
			return Ev::lift(m.statistics);
		});
	};
	typedef std::map<Ln::NodeId, Boss::Msg::PeerStatistics> Statistics;

	auto code = Ev::lift().then([&]() {

		/* Broadcast the db.  */
		return bus.raise(Boss::Msg::DbResource{db});
	}).then([&]() {

		/* Tell everyone we are now online.  */
		return bus.raise(Boss::Msg::InternetOnline{true});
	}).then([&]() {

		/* Create a dummy connected channel.  */
		return bus.raise(Boss::Msg::ListpeersAnalyzedResult{
			{A}, {}, {}, {}, false
		});
	}).then([&]() {

		/* Check the statistics.  */
		return get_stats();
	}).then([&](Statistics stats) {
		/* A should have an entry now.  */
		assert(stats.find(A) != stats.end());

		/* Tell the statistician that A no longer exists.  */
		return bus.raise(Boss::Msg::ListpeersAnalyzedResult{
			{}, {}, {}, {}, false
		});
	}).then([&]() {

		/* Check the statistics.  */
		return get_stats();
	}).then([&](Statistics stats) {
		/* The entry of A should now be gone.  */
		assert(stats.find(A) == stats.end());

		return Ev::lift(0);
	});
	return Ev::start(code);
}

