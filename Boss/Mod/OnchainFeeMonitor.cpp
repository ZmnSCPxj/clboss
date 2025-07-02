#include"Boss/Mod/OnchainFeeMonitor.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/OnchainFee.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/ManifestCommand.hpp"
#include"Boss/Msg/ProvideStatus.hpp"
#include"Boss/Msg/SolicitStatus.hpp"
#include"Boss/Msg/Timer10Minutes.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Stats/RunningMean.hpp"
#include"Util/make_unique.hpp"
#include<vector>

namespace {

/* Keep two weeks worth of data.  */
auto constexpr max_num_samples = std::size_t(2016);

auto const initialization = R"INIT(
DROP TABLE IF EXISTS "OnchainFeeMonitor_meanfee";
CREATE TABLE IF NOT EXISTS "OnchainFeeMonitor_samples"
	( id INTEGER PRIMARY KEY AUTOINCREMENT
	, data SAMPLE
	);
CREATE INDEX IF NOT EXISTS "OnchainFeeMonitor_samples_idx"
    ON "OnchainFeeMonitor_samples"(data);
)INIT";

/* Initial data we will load into OnchainFeeMonitor_samples,
 * so that new users have something reasonable to start out
 * with.  */
std::uint32_t const initial_sample_data[] =
{
    253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253
};

static_assert( Boss::Mod::OnchainFeeMonitor::num_initial_samples == ( sizeof(initial_sample_data)
								    / sizeof(initial_sample_data[0])
		       )
	       , "initial_sample_data number of entries must equal num_initial_samples"
	     );

/* What percentile to use to judge low and high.
 * `mid_percentile` is used in the initial starting check.
 * `hi_to_lo_percentile` is used when we are currently in
 * "high fees" mode, and if the onchain fees get less than
 * or equal that rate, we move to "low fees" mode.
 * `lo_to_hi_percentile` is used when we are currently in
 * "low fees" mode.
 */
auto constexpr hi_to_lo_percentile = double(17);
auto constexpr mid_percentile = double(20);
auto constexpr lo_to_hi_percentile = double(23);

}

namespace Boss { namespace Mod {

struct PercentileFeerates {
	double h2l;
	double mid;
	double l2h;

	PercentileFeerates()
        : h2l(-1), mid(-1), l2h(-1) {}
};

class OnchainFeeMonitor::Impl {
private:
	S::Bus& bus;
	Boss::Mod::Waiter& waiter;
	Boss::Mod::Rpc *rpc;
	Sqlite3::Db db;

	bool is_low_fee_flag;
	std::unique_ptr<double> last_feerate;

	void start() {
		bus.subscribe<Msg::DbResource
			     >([this](Msg::DbResource const& m) {
			db = m.db;
			/* Set up table if not yet present.  */
			return db.transact().then([this](Sqlite3::Tx tx) {
				tx.query_execute(initialization);
				initialize_sample_data(tx);
				tx.commit();
				return Ev::lift();
			});
		});
		bus.subscribe<Msg::Init>([this](Msg::Init const& ini) {
			rpc = &ini.rpc;
			return Boss::concurrent(on_init());
		});
		bus.subscribe<Msg::Timer10Minutes>([this](Msg::Timer10Minutes const&) {
			return on_timer();
		});
		/* Report status.  */
		bus.subscribe<Msg::SolicitStatus
			     >([this](Msg::SolicitStatus const& _) {
			if (!db)
				return Ev::lift();
			return db.transact().then([this](Sqlite3::Tx tx) {
				auto feerates = get_percentile_feerates(tx);
				tx.commit();

				auto status = Json::Out()
					.start_object()
						.field("hi_to_lo", feerates.h2l)
						.field("init_mid", feerates.mid)
						.field("lo_to_hi", feerates.l2h)
						.field( "last_feerate_perkw"
						      , last_feerate ? *last_feerate : -1.0
						      )
						.field( "judgment"
						      , std::string(is_low_fee_flag ? "low fees" : "high fees")
						      )
					.end_object()
					;
				return bus.raise(Msg::ProvideStatus{
					"onchain_feerate",
					std::move(status)
				});
			});
		});

		/* Manifestation.  */
		bus.subscribe<Msg::Manifestation
			     >([this](Msg::Manifestation const&) {
			return bus.raise(Msg::ManifestCommand{
				"clboss-feerates", "",
				"Show onchain feerate thresholds and current judgment.",
				false
			});
		});

		/* Command handler.  */
		bus.subscribe<Msg::CommandRequest
			     >([this](Msg::CommandRequest const& req) {
			if (req.command != "clboss-feerates")
				return Ev::lift();

			auto id = req.id;
			if (!db) {
				auto result = Json::Out()
					.start_object()
						.field("hi_to_lo", -1.0)
						.field("init_mid", -1.0)
						.field("lo_to_hi", -1.0)
						.field("last_feerate_perkw", last_feerate ? *last_feerate : -1.0)
						.field("judgment", std::string(is_low_fee_flag ? "low" : "high"))
					.end_object();
				return bus.raise(Msg::CommandResponse{id, std::move(result)});
			}

			return db.transact().then([this](Sqlite3::Tx tx) {
				auto feerates = get_percentile_feerates(tx);
				tx.commit();
				return Ev::lift(feerates);
			}).then([this, id](PercentileFeerates feerates) {
				auto result = Json::Out()
					.start_object()
						.field("hi_to_lo", feerates.h2l)
						.field("init_mid", feerates.mid)
						.field("lo_to_hi", feerates.l2h)
						.field("last_feerate_perkw", last_feerate ? *last_feerate : -1.0)
						.field("judgment", std::string(is_low_fee_flag ? "low" : "high"))
					.end_object();
				return bus.raise(Msg::CommandResponse{id, std::move(result)});
			});
		});
	}

	size_t get_num_samples(Sqlite3::Tx& tx) {
		auto fetch = tx.query(R"QRY(
		SELECT COUNT(*)
		  FROM "OnchainFeeMonitor_samples"
		     ;
		)QRY")
			.execute()
			;
		auto num_current_samples = std::size_t();
		for (auto& r : fetch)
                  num_current_samples = r.get<std::size_t>(0);
                return num_current_samples;
        }

	void initialize_sample_data(Sqlite3::Tx& tx) {
          	size_t count = get_num_samples(tx);
		if (count >= num_initial_samples)
			return;
		for (auto i = count; i < num_initial_samples; ++i) {
			tx.query(R"QRY(
			INSERT INTO "OnchainFeeMonitor_samples"(data)
			VALUES(:data);
			)QRY")
				.bind(":data", initial_sample_data[i])
				.execute()
				;
		}
	}

	Ev::Io<void> on_init() {
		auto saved_feerate = std::make_shared<double>();

		return Ev::lift().then([this]() {
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
			last_feerate = std::move(feerate);

			/* Now access the db and check if it is lower or
			 * higher than the mid percentile, to know our current
			 * low/high flag.  */
			return db.transact().then([ this
						  , saved_feerate
						  ](Sqlite3::Tx tx) {
				auto feerates = get_percentile_feerates(tx);
				tx.commit();
				is_low_fee_flag = *saved_feerate < feerates.mid;
				return report_fee("Init", feerates);
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
				   ).then([](Jsmn::Object res) {
			auto failed = []() {
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

	void add_sample(Sqlite3::Tx& tx, double feerate_sample) {
		tx.query(R"QRY(
		INSERT INTO "OnchainFeeMonitor_samples"
		      ( data)
		VALUES(:data);
		)QRY")
			.bind(":data", feerate_sample)
			.execute()
			;

		check_num_samples(tx);
	}

	void check_num_samples(Sqlite3::Tx& tx) {
          	size_t num = get_num_samples(tx);
		if (num > max_num_samples) {
			/* We did!  Delete old ones.  */

			/** FIXME: This could have been done with
			 * `DELETE ... ORDER BY id LIMIT :limit`,
			 * but not all OSs (presumably FreeBSD or
			 * MacOS) have a default SQLITE3 with
			 * `SQLITE_ENABLE_UPDATE_DELETE_LIMIT`
			 * enabled.
			 * The real fix is to put SQLITE3 into our
			 * `external/` dir where we can strictly
			 * control the flags it gets compiled
			 * with.
			 */
			auto ids = std::vector<std::uint64_t>();
			auto fetch = tx.query(R"QRY(
			SELECT id FROM "OnchainFeeMonitor_samples"
			 ORDER BY id
			 LIMIT :limit
			     ;
			)QRY")
				.bind(":limit", num - max_num_samples)
				.execute()
				;
			for (auto& r : fetch)
				ids.push_back(r.get<std::uint64_t>(0));
			for (auto id : ids) {
				tx.query(R"QRY(
				DELETE FROM "OnchainFeeMonitor_samples"
				 WHERE id = :id
				     ;
				)QRY")
					.bind(":id", id)
					.execute()
					;
			}
		}
	}

	PercentileFeerates get_percentile_feerates(Sqlite3::Tx& tx) {
		PercentileFeerates feerates;
		feerates.h2l = get_feerate_at_percentile(
			tx, hi_to_lo_percentile
			);
		feerates.mid = get_feerate_at_percentile(
			tx, mid_percentile
			);
		feerates.l2h = get_feerate_at_percentile(
			tx, lo_to_hi_percentile
			);
		return feerates;
	}

	double
	get_feerate_at_percentile(Sqlite3::Tx& tx, double percentile) {
          	/* Use the actual number of samples in the collection.
		 * This allows us to use the collection while it fills. */
          	size_t num_current_samples = get_num_samples(tx);

                /* The table should never be empty, but if it is it's
                 * easiest to just return a (conservative) reasonable
                 * value until we gather some data ...
                 */
                if (num_current_samples == 0)
                  return 253.0;

		auto index = std::size_t(
			double(num_current_samples) * percentile / 100.0
		);
		if (index < 0)
			index = std::size_t(0);
		else if (index >= num_current_samples)
			index = num_current_samples - 1;

		auto fetch = tx.query(R"QRY(
		SELECT data FROM "OnchainFeeMonitor_samples"
		 ORDER BY data
		 LIMIT 1 OFFSET :index
		)QRY")
			.bind(":index", index)
			.execute();
		auto rv = double();
		for (auto& r : fetch)
			rv = r.get<double>(0);

		return rv;
	}

	Ev::Io<void> report_fee(char const* msg, PercentileFeerates const& feerates) {
		return Boss::log( bus, Debug
				, "OnchainFeeMonitor: %s: (%.0f, %.0f, %.0f): %.0f: %s fees."
				  , msg
				  , feerates.h2l
				  , feerates.mid
				  , feerates.l2h
				  , last_feerate ? *last_feerate : -1.0
				  , is_low_fee_flag ? "low" : "high"
				).then([this]() {
			if (last_feerate) {
				return bus.raise(Msg::OnchainFee{
					is_low_fee_flag,
					Util::make_unique<double>(*last_feerate)
				});
			} else {
				return bus.raise(Msg::OnchainFee{
					is_low_fee_flag,
					nullptr
				});
			}
		});
	}

	Ev::Io<void> on_timer() {
		auto saved_feerate = std::make_shared<double>();

		return get_feerate().then([ this
					  , saved_feerate
					  ](std::unique_ptr<double> feerate) {
			if (!feerate)
				return report_fee("Periodic: fee unknown, retaining", PercentileFeerates());

			*saved_feerate = *feerate;
			last_feerate = std::move(feerate);

			return db.transact().then([ this
						  , saved_feerate
						  ](Sqlite3::Tx tx) {
				add_sample(tx, *saved_feerate);

				/* Apply hysteresis.
				 * It is possible for the lo_to_hi and
				 * hi_to_lo to be equal, if the feerates
				 * have been relatively flat for a long
				 * time.
				 * The point of this hysteresis is to
				 * avoid waffling between "low" and "high"
				 * fees too often, so the low-to-high
				 * transition is compared with >, but the
				 * high-to-low transition is compared with
				 * <=, since it is possible that both
				 * transition points are equal.
				 *
				 * One particular stable point is at the
				 * minimum allowed feerate, which is why
				 * it is the low-to-high transition that
				 * is compared with > --- otherwise if
				 * the blockchain stabilizes at the
				 * minimum feerate for the past two weeks,
				 * then the feerate will be at the minimum
				 * and comparing with >= would cause us
				 * to think "high fees" even though we are
				 * at minimum.
				 */
				auto feerates = get_percentile_feerates(tx);
				if (is_low_fee_flag) {
					if (*saved_feerate > feerates.l2h)
						is_low_fee_flag = false;
				} else {
					if (*saved_feerate <= feerates.h2l)
						is_low_fee_flag = true;
				}
				tx.commit();
				return report_fee("Periodic", feerates);
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
				auto feerates = get_percentile_feerates(tx);
				tx.commit();
				is_low_fee_flag = *saved_feerate <= feerates.mid;
				return report_fee("Init", feerates);
			});
		});
	}

public:
	explicit
	Impl( S::Bus& bus_
	    , Boss::Mod::Waiter& waiter_
	    ) : bus(bus_), waiter(waiter_)
	      , rpc(nullptr), is_low_fee_flag(false)
	      , last_feerate(nullptr)
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
