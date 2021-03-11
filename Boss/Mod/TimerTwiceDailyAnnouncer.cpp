#include"Boss/Mod/TimerTwiceDailyAnnouncer.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/Timer10Minutes.hpp"
#include"Boss/Msg/TimerTwiceDaily.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/now.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Util/make_unique.hpp"

namespace {

auto constexpr half_day = double(12 * 60 * 60);

}

namespace Boss { namespace Mod {

class TimerTwiceDailyAnnouncer::Impl {
private:
	S::Bus& bus;
	Sqlite3::Db db;

	void start() {
		bus.subscribe<Msg::Init
			     >([this](Msg::Init const& init) {
			db = init.db;
			return initialize_db()
			     + Boss::concurrent(check())
			     ;
		});
		bus.subscribe<Msg::Timer10Minutes
			     >([this](Msg::Timer10Minutes const& _) {
			return check();
		});
	}
	Ev::Io<void> initialize_db() {
		return db.transact().then([](Sqlite3::Tx tx) {
			tx.query_execute(R"QRY(
			CREATE TABLE IF NOT EXISTS "TimerTwiceDailyAnnouncer"
			     ( id INTEGER PRIMARY KEY
			     , prevtime REAL NOT NULL
			     );
			INSERT OR IGNORE INTO "TimerTwiceDailyAnnouncer"
			VALUES(1, 0.0);
			)QRY");
			tx.commit();

			return Ev::lift();
		});
	}
	Ev::Io<void> check() {
		return db.transact().then([this](Sqlite3::Tx tx) {
			auto act = Ev::lift();

			auto fetch = tx.query(R"QRY(
			SELECT prevtime FROM "TimerTwiceDailyAnnouncer";
			)QRY")
				.execute();
			for (auto& r : fetch) {
				auto now = Ev::now();
				auto prevtime = r.get<double>(0);
				if (prevtime + half_day < now) {
					tx.query(R"QRY(
					UPDATE "TimerTwiceDailyAnnouncer"
					   SET prevtime = :now;
					)QRY")
						.bind(":now", now)
						.execute()
						;
					act = Boss::concurrent(raise());
				}
			}
			tx.commit();

			return act;
		});
	}
	Ev::Io<void> raise() {
		return Ev::lift().then([this]() {
			return Boss::log( bus, Debug
					, "TimerTwiceDailyAnnouncer: Announce."
					);
		}).then([this]() {
			return bus.raise(Msg::TimerTwiceDaily{});
		});
	}

public:
	explicit
	Impl( S::Bus& bus_
	    ) : bus(bus_)
	      { start(); }
};

TimerTwiceDailyAnnouncer::TimerTwiceDailyAnnouncer(TimerTwiceDailyAnnouncer&&)
	=default;
TimerTwiceDailyAnnouncer::~TimerTwiceDailyAnnouncer()
	=default;

TimerTwiceDailyAnnouncer::TimerTwiceDailyAnnouncer(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) {}

}}
