#include"Boss/Mod/PeerJudge/AgeTracker.hpp"
#include"Boss/Msg/ChannelCreation.hpp"
#include"Boss/Msg/ChannelDestruction.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/concurrent.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"

namespace Boss { namespace Mod { namespace PeerJudge {

void AgeTracker::start() {
	bus.subscribe<Msg::DbResource
		     >([this](Msg::DbResource const& m) {
		db = m.db;
		return db.transact().then([](Sqlite3::Tx tx) {
			tx.query_execute(R"SQL(
			CREATE TABLE IF NOT EXISTS
			       "PeerJudge_ages"
			       ( node TEXT PRIMARY KEY
			       , time REAL
			       );
			)SQL");
			tx.commit();

			return Ev::lift();
		});
	});
	bus.subscribe<Msg::ChannelCreation
		     >([this](Msg::ChannelCreation const& m) {
		auto const& node = m.peer;
		auto time = get_now();
		return Boss::concurrent(update(node, time));
	});
	bus.subscribe<Msg::ChannelDestruction
		     >([this](Msg::ChannelDestruction const& m) {
		auto const& node = m.peer;
		auto time = get_now();
		return Boss::concurrent(update(node, time));
	});
}
Ev::Io<void> AgeTracker::wait_for_db() {
	return Ev::yield().then([this]() {
		if (!db)
			return wait_for_db();
		return Ev::lift();
	});
}
Ev::Io<void> AgeTracker::update(Ln::NodeId node, double time) {
	return wait_for_db().then([this]() {
		return db.transact();
	}).then([node, time](Sqlite3::Tx tx) {
		tx.query(R"SQL(
		INSERT OR REPLACE
		  INTO "PeerJudge_ages"
		VALUES( :node
		      , :time
		      )
		)SQL")
			.bind(":node", std::string(node))
			.bind(":time", time)
			.execute()
			;
		tx.commit();
		return Ev::lift();
	});
}

Ev::Io<double> AgeTracker::get_min_age(Ln::NodeId node) {
	auto now = get_now();
	return wait_for_db().then([this]() {
		return db.transact();
	}).then([now, node](Sqlite3::Tx tx) {
		auto fetch = tx.query(R"SQL(
		SELECT time
		  FROM "PeerJudge_ages"
		 WHERE node = :node
		     ;
		)SQL")
			.bind(":node", std::string(node))
			.execute()
			;
		auto time = double(0.0);
		for (auto& r : fetch)
			time = r.get<double>(0);
		return Ev::lift(now - time);
	});
}

}}}
