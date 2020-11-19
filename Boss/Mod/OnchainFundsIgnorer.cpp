#include"Boss/Mod/OnchainFundsIgnorer.hpp"
#include"Boss/Msg/CommandFail.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/ManifestCommand.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/ProvideStatus.hpp"
#include"Boss/Msg/RequestGetOnchainIgnoreFlag.hpp"
#include"Boss/Msg/ResponseGetOnchainIgnoreFlag.hpp"
#include"Boss/Msg/SolicitStatus.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/foreach.hpp"
#include"Ev/now.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Util/make_unique.hpp"
#include<vector>

namespace Boss { namespace Mod {

class OnchainFundsIgnorer::Impl {
private:
	S::Bus& bus;
	Sqlite3::Db db;

	std::vector<void*> pending_requesters;

	void start() {
		bus.subscribe<Msg::DbResource
			     >([this](Msg::DbResource const& m) {
			db = m.db;
			auto pending = std::move(pending_requesters);
			return init_db()
			     + Ev::foreach( [this](void* requester) {
						return get_flag(requester);
					    }
					  , std::move(pending)
					  );
		});
		bus.subscribe<Msg::RequestGetOnchainIgnoreFlag
			     >([this
			       ](Msg::RequestGetOnchainIgnoreFlag const& m) {
			if (!db) {
				pending_requesters.push_back(m.requester);
				return Ev::lift();
			}
			return get_flag(m.requester);
		});

		/* Manifestation.  */
		bus.subscribe<Msg::Manifestation
			     >([this](Msg::Manifestation const& _) {
			return bus.raise(Msg::ManifestCommand{
				"clboss-ignore-onchain", "[hours]",
				"Ignore onchain funds for {hours} "
				"(default 24 hours).",
				false
			}) + bus.raise(Msg::ManifestCommand{
				"clboss-notice-onchain", "",
				"Cancel previous clboss-ignore-onchain and "
				"resume managing onchain funds.",
				false
			});
		});

		/* Command handling.  */
		bus.subscribe<Msg::CommandRequest
			     >([this](Msg::CommandRequest const& req) {
			auto id = req.id;
			auto succeed = [this, id]() {
				return bus.raise(Msg::CommandResponse{
					id, Json::Out::empty_object()
				});
			};

			auto act = Ev::lift();
			if (req.command == "clboss-ignore-onchain") {
				auto paramfail = [this, id]() {
					return bus.raise(Msg::CommandFail{
						id, -32602,
						"Parameter failure",
						Json::Out::empty_object()
					});
				};

				auto hours = double(24.0);

				auto hours_j = Jsmn::Object();
				auto params = req.params;
				if (params.is_object()) {
					if (params.size() > 1)
						return paramfail();
					if (params.size() == 1) {
						if (!params.has("hours"))
							return paramfail();
						hours_j = params["hours"];
					}
				} else if (params.is_array()) {
					if (params.size() > 1)
						return paramfail();
					if (params.size() == 1)
						hours_j = params[0];
				} else
					return paramfail();
				if (!hours_j.is_null()) {
					if (!hours_j.is_number())
						return paramfail();
					hours = (double) hours_j;
				}

				act += set_disableuntil( Ev::now()
						       + (hours * 3600)
						       );
				act += Boss::log( bus, Info
						, "OnchainFundsIgnorer: "
						  "Will ignore onchain funds "
						  "for %f hours starting now."
						, hours
						);
				act += succeed();

			} else if (req.command == "clboss-notice-onchain") {
				act += set_disableuntil(0);
				act += Boss::log( bus, Info
						, "OnchainFundsIgnorer: "
						  "Will notice onchain funds."
						);
				act += succeed();
			}
			return act;
		});

		/* clboss-status */
		bus.subscribe<Msg::SolicitStatus
			     >([this](Msg::SolicitStatus const& _) {
			return Ev::lift().then([this]() {
				return get_disableuntil();
			}).then([this](double disableuntil) {
				auto now = Ev::now();
				auto ignore = (now < disableuntil);
				auto comment = std::string();
				if (ignore)
					comment = "`clboss-notice-onchain` "
						  "to resume managing "
						  "onchain funds.";
				else
					comment = "`clboss-ignore-onchain` "
						  "to ignore onchain funds "
						  "temporarily.";

				auto stats = Json::Out()
					.start_object()
						.field( "status"
						      , ignore ?
							"ignore" : "notice"
						      )
						.field("now", now)
						.field( "disable_until"
						      , disableuntil
						      )
						.field("comment", comment)
					.end_object()
					;
				return bus.raise(Msg::ProvideStatus{
					"should_monitor_onchain_funds",
					std::move(stats)
				});
			});
		});
	}

	Ev::Io<void> init_db() {
		return db.transact().then([](Sqlite3::Tx tx) {
			tx.query_execute(R"QRY(
			CREATE TABLE IF NOT EXISTS "OnchainFundsIgnorer"
			     ( id INTEGER PRIMARY KEY
			     , disableuntil REAL NOT NULL
			     );
			INSERT OR IGNORE INTO "OnchainFundsIgnorer"
			VALUES(1, 0.0);
			)QRY");
			tx.commit();
			return Ev::lift();
		});
	}
	Ev::Io<void> get_flag(void* requester) {
		return get_disableuntil().then([requester, this
					       ](double disableuntil) {
			auto now = Ev::now();

			auto msg = Msg::ResponseGetOnchainIgnoreFlag();
			msg.ignore = (now < disableuntil);
			if (msg.ignore)
				msg.seconds = disableuntil - now;
			else
				msg.seconds = 0;
			msg.requester = requester;
			return bus.raise(msg);
		});
	}
	Ev::Io<double> get_disableuntil() {
		return db.transact().then([this](Sqlite3::Tx tx) {
			auto rv = double();

			auto fetch = tx.query(R"QRY(
			SELECT disableuntil FROM "OnchainFundsIgnorer";
			)QRY").execute();
			for (auto& r : fetch) {
				rv = r.get<double>(0);
				break;
			}
			tx.commit();

			return Ev::lift(rv);
		});
	}
	Ev::Io<void> set_disableuntil(double disableuntil) {
		return db.transact().then([disableuntil](Sqlite3::Tx tx) {
			tx.query(R"QRY(
			UPDATE "OnchainFundsIgnorer"
			   SET disableuntil = :disableuntil;
			)QRY")
				.bind(":disableuntil", disableuntil)
				.execute()
				;
			tx.commit();
			return Ev::lift();
		});
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	Impl(S::Bus& bus_) : bus(bus_) { start(); }
};

OnchainFundsIgnorer::OnchainFundsIgnorer(OnchainFundsIgnorer&&) =default;
OnchainFundsIgnorer::~OnchainFundsIgnorer() =default;

OnchainFundsIgnorer::OnchainFundsIgnorer(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
