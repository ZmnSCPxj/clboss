#include"Boss/Mod/OnchainFeeMonitor.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/OnchainFee.hpp"
#include"Boss/Msg/TimerRandomHourly.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Stats/RunningMean.hpp"
#include"Util/make_unique.hpp"

namespace {

/* Works for Bitcoin, so...  */
auto const minimum_feerate = double(253);

auto const hysteresis_percent = double(20);

auto const initialization = R"INIT(
CREATE TABLE IF NOT EXISTS "OnchainFeeMonitor_meanfee"
	( id INTEGER PRIMARY KEY
	, mean DOUBLE
	, samples INTEGER
	);
INSERT OR IGNORE INTO "OnchainFeeMonitor_meanfee" VALUES
	( 0
	, 253
	, 1
	);
)INIT";

auto const q_get = R"QRY(
SELECT mean, samples FROM "OnchainFeeMonitor_meanfee"
 WHERE id = 0
     ;
)QRY";

auto const q_set = R"QRY(
UPDATE "OnchainFeeMonitor_meanfee"
   SET mean = :mean
     , samples = :samples
 WHERE id = 0
     ;
)QRY";

}

namespace Boss { namespace Mod {

class OnchainFeeMonitor::Impl {
private:
	S::Bus& bus;
	Boss::Mod::Waiter& waiter;
	Boss::Mod::Rpc *rpc;
	Sqlite3::Db db;

	bool is_low_fee_flag;

	void start() {
		bus.subscribe<Msg::Init>([this](Msg::Init const& ini) {
			rpc = &ini.rpc;
			db = ini.db;
			return on_init();
		});
		bus.subscribe<Msg::TimerRandomHourly>([this](Msg::TimerRandomHourly const&) {
			return on_timer();
		});
	}

	Ev::Io<void> on_init() {
		auto saved_feerate = std::make_shared<double>();

		/* Set up table if not yet present.  */
		return db.transact().then([this](Sqlite3::Tx tx) {
			tx.query_execute(initialization);
			tx.commit();

			/* Now query the feerate.  */
			return get_feerate();
		}).then([ this
			, saved_feerate
			](std::unique_ptr<double> feerate) {
			/* If feerate is unknown, just assume we are at
			 * a high-feerate time.
			 */
			if (!feerate)
				return Boss::log( bus, Debug
						, "OnchainFeeMonitor: Init: "
						  "Fee unknown."
						).then([this]() {
					return Boss::concurrent(retry());
				});

			/* Feerate is known, save it in the shared variable.  */
			*saved_feerate = *feerate;

			/* Now access the db and check if it is lower or
			 * higher than the mean, to know our current
			 * low/high flag.  */
			return db.transact().then([ this
						  , saved_feerate
						  ](Sqlite3::Tx tx) {
				auto mean = update_mean( std::move(tx)
						       , *saved_feerate
						       );
				is_low_fee_flag = *saved_feerate < mean;
				return report_fee("Init");
			});
		});
	}

	/* Use std::unique_ptr<double> as a sort of Optional Real.  */
	Ev::Io<std::unique_ptr<double>> get_feerate() {
		return rpc->command("feerates"
				   , Json::Out()
					.start_object()
						.field("style", std::string("perkw"))
					.end_object()
				   ).then([this](Jsmn::Object res) {
			auto failed = [this]() {
				return Ev::lift<std::unique_ptr<double>>(
					nullptr
				);
			};
			if (!res.is_object() || !res.has("perkw"))
				return failed();
			auto perkw = res["perkw"];
			if (!perkw.is_object() || !perkw.has("opening"))
				return failed();
			auto opening = perkw["opening"];
			if (!opening.is_number())
				return failed();
			auto value = (double) opening;
			return Ev::lift(
				Util::make_unique<double>(value)
			);
		});
	}

	double update_mean(Sqlite3::Tx tx, double feerate_sample) {
		/* Recover the RunningMean.  */
		auto fetch = tx.query(q_get).execute();
		auto memo = Stats::RunningMean::Memo();
		for (auto& r : fetch) {
			memo.mean = r.get<double>(0);
			memo.samples = r.get<std::size_t>(1);
		}
		auto running_mean = Stats::RunningMean(memo);

		/* Update it.  */
		running_mean.sample(feerate_sample);
		memo = running_mean.get_memo();
		tx.query(q_set)
			.bind(":mean", memo.mean)
			.bind(":samples", memo.samples)
			.execute();
		tx.commit();

		/* Return mean.  */
		return running_mean.get();
	}

	Ev::Io<void> report_fee(char const* msg) {
		return Boss::log( bus, Debug
				, "OnchainFeeMonitor: %s: %s fees."
				, msg
				, is_low_fee_flag ? "low" : "high"
				).then([this]() {
			return bus.raise(Msg::OnchainFee{is_low_fee_flag});
		});
	}

	Ev::Io<void> on_timer() {
		auto saved_feerate = std::make_shared<double>();

		return get_feerate().then([ this
					  , saved_feerate
					  ](std::unique_ptr<double> feerate) {
			if (!feerate)
				return report_fee("Hourly: fee unknown, retaining");

			*saved_feerate = *feerate;

			return db.transact().then([ this
						  , saved_feerate
						  ](Sqlite3::Tx tx) {
				auto mean = update_mean( std::move(tx)
						       , *saved_feerate
						       );

				/* Make it so minimum fee of 253 is 0.  */
				auto mean_base = mean - minimum_feerate;
				auto feerate_base = *saved_feerate - minimum_feerate;
				/* Should not happen.  */
				if (mean_base < 0) mean_base = 0;
				if (feerate_base < 0) feerate_base = 0;

				/* Apply hysteresis.  */
				if (is_low_fee_flag) {
					auto ref = mean_base
						 * (100 + hysteresis_percent)
						 / 100
						 ;
					if (feerate_base >= ref)
						is_low_fee_flag = false;
				} else {
					auto ref = mean_base
						 * (100 - hysteresis_percent)
						 / 100
						 ;
					if (feerate_base <= ref)
						is_low_fee_flag = true;
				}
				return report_fee("Hourly");
			});
		});
	}

	/* Loop until we get fees.  */
	Ev::Io<void> retry() {
		auto saved_feerate = std::make_shared<double>();
		return waiter.wait(30).then([this]() {
			return get_feerate();
		}).then([this, saved_feerate
			](std::unique_ptr<double> feerate) {
			if (!feerate)
				return Boss::log( bus, Debug
						, "OnchainFeeMonitor: "
						  "Init retried: "
						  "Fee still unknown."
						).then([this]() {
					return retry();
				});

			return db.transact().then([ this
						  , saved_feerate
						  ](Sqlite3::Tx tx) {
				auto mean = update_mean( std::move(tx)
						       , *saved_feerate
						       );
				is_low_fee_flag = *saved_feerate < mean;
				return report_fee("Init retried");
			});
		});
	}

public:
	explicit
	Impl( S::Bus& bus_
	    , Boss::Mod::Waiter& waiter_
	    ) : bus(bus_), waiter(waiter_)
	      , rpc(nullptr), is_low_fee_flag(false)
	      {
		start();
	}

	bool is_low_fee() const {
		return is_low_fee_flag;
	}
};

OnchainFeeMonitor::OnchainFeeMonitor( S::Bus& bus
				    , Boss::Mod::Waiter& waiter
				    )
	: pimpl(Util::make_unique<Impl>(bus, waiter)) { }
OnchainFeeMonitor::OnchainFeeMonitor(OnchainFeeMonitor&& o)
	: pimpl(std::move(o.pimpl)) { }
OnchainFeeMonitor::~OnchainFeeMonitor() { }

bool OnchainFeeMonitor::is_low_fee() const {
	return pimpl->is_low_fee();
}

}}
