#include"Boss/Mod/ComplainerByLowSuccessPerDay.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/ProbeActively.hpp"
#include"Boss/Msg/RaisePeerComplaint.hpp"
#include"Boss/Msg/RequestSelfUptime.hpp"
#include"Boss/Msg/ResponseSelfUptime.hpp"
#include"Boss/Msg/SolicitPeerComplaints.hpp"
#include"Boss/Msg/TimerRandomHourly.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
#include"Ev/now.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Stats/ReservoirSampler.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"
#include<set>

namespace {

/* Nodes with less than this success_per_day should be complained about.  */
auto constexpr min_success_per_day = double(0.25);
/* Nodes with less than or equal to this success_per_day should be actively
 * probed to try to boost that metric; maybe the metric is low only because
 * the random active probing rolled against it too often.
 */
auto constexpr warn_success_per_day = double(1.0);
/* Maximum number of nodes we will actively probe.  */
auto constexpr max_probes = std::size_t(10);

/* If our own uptime in the past two weeks is below this, do not complain
 * about any peers.  */
auto constexpr min_self_uptime = double(0.35);

/* The minimum amount of time, in seconds, this module has been running
 * before it actually starts complaining.
 * This allows the module to trigger active probes to increase the metric
 * before it starts complaining about low-metric nodes.
 */
auto constexpr min_module_lifetime = double(7 * 24 * 60 * 60.0); // 7 days

}

namespace Boss { namespace Mod {

class ComplainerByLowSuccessPerDay::Impl {
private:
	S::Bus& bus;

	ModG::ReqResp< Msg::RequestSelfUptime
		     , Msg::ResponseSelfUptime
		     > uptime_rr;

	double start_time;

	/* Set of nodes that we need to probe actively because their
	 * success_per_day is fairly low.
	 */
	std::set<Ln::NodeId> pending_probe;

	void start() {
		start_time = 999999999999999999999.0;
		bus.subscribe< Msg::DbResource
			     >([this](Msg::DbResource const& m) {
			auto db = m.db;
			return db.transact().then([this](Sqlite3::Tx tx) {
				tx.query_execute(R"QRY(
				CREATE TABLE IF NOT EXISTS "ComplainerByLowSuccessPerDay"
				     ( id INTEGER PRIMARY KEY
				     , start_time
				     );
				)QRY");
				tx.query(R"QRY(
				INSERT OR IGNORE INTO "ComplainerByLowSuccessPerDay"
				VALUES( 1
				      , :now
				      );
				)QRY")
					.bind(":now", Ev::now())
					.execute()
					;
				auto fetch = tx.query(R"QRY(
				SELECT start_time FROM "ComplainerByLowSuccessPerDay"
				 WHERE id = 1
				     ;
				)QRY")
					.execute()
					;
				for (auto& r : fetch)
					start_time = r.get<double>(0);

				tx.commit();
				return Boss::log( bus, Debug
						, "ComplainerByLowSuccessPerDay: "
						  "This module was originally started on "
						  "%f; it is %f seconds since then."
						, start_time
						, Ev::now() - start_time
						);
			});
		});

		bus.subscribe< Msg::SolicitPeerComplaints
			     >([this](Msg::SolicitPeerComplaints const& metrics) {
			return Ev::lift().then([this]() {
				return uptime_rr.execute(Msg::RequestSelfUptime());
			}).then([this, metrics] (Msg::ResponseSelfUptime uptime_data) {
				auto uptime = uptime_data.weeks2;

				auto act = Ev::lift();

				auto logged_nonoperation = false;
				auto nonoperation = [ &act
						    , &logged_nonoperation
						    , this
						    ](std::string const& s) {
					if (logged_nonoperation)
						return;
					logged_nonoperation = true;
					act += Boss::log( bus, Debug
							, "ComplainerByLowSuccessPerDay: "
							  "Will not complain: %s"
							, s.c_str()
							);
				};

				for (auto const& m : metrics.weeks2) {
					auto success_per_day = m.second.success_per_day;
					/* Mark those below the warning level.  */
					if (success_per_day <= warn_success_per_day)
						pending_probe.insert(m.first);

					if (uptime < min_self_uptime) {
						nonoperation( std::string()
							    + "This node uptime is only "
							    + Util::stringify(uptime)
							    + ", minimum is "
							    + Util::stringify(min_self_uptime)
							    + "."
							    );
						continue;
					}
					auto run_time = Ev::now() - start_time;
					if (run_time < min_module_lifetime) {
						nonoperation( std::string()
							    + "Module has been running for "
							    + Util::stringify(run_time)
							    + ", minimum is "
							    + Util::stringify(
								min_module_lifetime
							      )
							    + "."
							    );
						continue;
					}

					auto adjusted_success_per_day =
						success_per_day / uptime;

					if (adjusted_success_per_day >= min_success_per_day)
						continue;

					auto reason = std::string()
						    + "ComplainerByLowSuccessPerDay: "
						    + "Payment attempts "
						    + "(probes + actual payments) "
						    + "that reach destination is "
						    + Util::stringify(success_per_day)
						    + "/day, adjusted for our 2-week uptime "
						    + "(" + Util::stringify(uptime) + ") "
						    + "is "
						    + Util::stringify(
							adjusted_success_per_day
						      )
						    + "/day, minimum adjusted is "
						    + Util::stringify(min_success_per_day)
						    + "/day"
						    ;

					act += bus.raise(Msg::RaisePeerComplaint{
						m.first,
						std::move(reason)
					});
				}

				return act;
			});
		});

		bus.subscribe< Msg::TimerRandomHourly
			     >([this](Msg::TimerRandomHourly const& _) {
			return Ev::lift().then([this]() {
				/* Probe only up to max_probes.  */
				auto sampler = Stats::ReservoirSampler<Ln::NodeId>(
					max_probes
				);
				for (auto const& node : pending_probe)
					sampler.add(node, 1.0, Boss::random_engine);
				auto selected = std::move(sampler).finalize();

				/* Trigger the active probes.  */
				auto act = Ev::lift();
				for (auto const& node : selected) {
					/* Remove from pending.  */
					auto it = pending_probe.find(node);
					pending_probe.erase(it);

					/* Raise the message.  */
					act += Boss::concurrent(bus.raise(Msg::ProbeActively{
						node
					}));
				}

				return act;
			});
		});
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	explicit
	Impl(S::Bus& bus_
	    ) : bus(bus_)
	      , uptime_rr( bus_
			 , [](Msg::RequestSelfUptime& m, void* p) {
				m.requester = p;
			   }
			 , [](Msg::ResponseSelfUptime& m) {
				return m.requester;
			   }
			 )
	      { start();  }
};

ComplainerByLowSuccessPerDay::~ComplainerByLowSuccessPerDay() =default;
ComplainerByLowSuccessPerDay::ComplainerByLowSuccessPerDay(ComplainerByLowSuccessPerDay&&)
	=default;

ComplainerByLowSuccessPerDay::ComplainerByLowSuccessPerDay(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
