#include"Boss/Mod/PeerJudge/Executioner.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ManifestOption.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/Option.hpp"
#include"Boss/Msg/ProvideStatus.hpp"
#include"Boss/Msg/SolicitStatus.hpp"
#include"Boss/Msg/TimerRandomHourly.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/foreach.hpp"
#include"Ev/now.hpp"
#include"Ev/yield.hpp"
#include"Json/Out.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Util/date.hpp"
#include"Util/format.hpp"
#include"Util/stringify.hpp"
#include<memory>
#include<string>
#include<utility>
#include<vector>

namespace {

auto constexpr default_enabled = bool(false);

auto constexpr channel_close_timeout = std::size_t(3 * 60);
auto const fee_negotiation_step = std::string("1");

}

namespace Boss { namespace Mod { namespace PeerJudge {

void Executioner::start() {
	enabled = default_enabled;

	/* Enable options.  */
	bus.subscribe< Msg::Manifestation
		     >([this](Msg::Manifestation const& _) {
		return bus.raise(Msg::ManifestOption{
			"clboss-auto-close", Msg::OptionType_Bool,
			Json::Out::direct(default_enabled),
			"Whether CLBOSS should automatically close "
			"bad channels (EXPERIMENTAL).  "
			"Set 'true' to enable, 'false' to disable."
		});
	});
	bus.subscribe<Msg::Option
		     >([this](Msg::Option const& o) {
		if (o.name != "clboss-auto-close")
			return Ev::lift();

		enabled = bool(o.value);
		if (enabled != default_enabled)
			return Boss::log( bus, Info
					, "PeerJudge: "
					  "Auto-close: %s."
					, enabled ? "enabled" :
						    "disabled"
					);
		return Ev::lift();
	});

	bus.subscribe< Msg::Init
		     >([this](Msg::Init const& m) {
		auto saved_db = m.db;
		auto saved_rpc = &m.rpc;
		return saved_db.transact().then([ saved_db
						, saved_rpc
						, this
						](Sqlite3::Tx tx) {
			tx.query_execute(R"QRY(
			CREATE TABLE IF NOT EXISTS
			       "PeerJudge_executed"
			     ( time REAL NOT NULL
			     , node TEXT NOT NULL
			     , reason TEXT NOT NULL
			     );
			CREATE INDEX IF NOT EXISTS
			       "PeerJudge_executed_timeindex"
			    ON "PeerJudge_executed"(time);
			)QRY");
			db = saved_db;
			rpc = saved_rpc;
			return Ev::lift();
		});
	});
	bus.subscribe< Msg::SolicitStatus
		     >([this](Msg::SolicitStatus const& _) {
		return wait_init().then([this]() {
			return db.transact();
		}).then([this](Sqlite3::Tx tx) {
			auto fetch = tx.query(R"QRY(
			SELECT time, node, reason
			  FROM "PeerJudge_executed"
			 ORDER BY time ASC;
			)QRY").execute();

			auto rv = Json::Out();
			auto arr = rv.start_array();
			for (auto& r : fetch) {
				auto time = r.get<double>(0);
				auto node = r.get<std::string>(1);
				auto reason = r.get<std::string>(2);

				auto text = Util::format( "%s: %s: %s"
							, Util::date(time)
								.c_str()
							, node.c_str()
							, reason.c_str()
							);
				arr.entry(text);
			}
			arr.end_array();

			tx.commit();

			return bus.raise(Msg::ProvideStatus{
				"closed_by_peer_judge",
				std::move(rv)
			});
		});
	});

	/* Check every hour.  */
	bus.subscribe< Msg::TimerRandomHourly
		     >([this](Msg::TimerRandomHourly const& _) {
		auto closes =
			std::make_shared<std::vector<std::pair< Ln::NodeId
							      , std::string
							      >>>();
		return wait_init().then([this, closes]() {
			auto closemap = get_close();
			auto const& unmanaged = get_unmanaged();

			auto message = std::string();
			if (enabled)
				message = "Will close: ";
			else
				message = "(disabled, will not actually "
					  "close, use "
					  "--clboss-auto-close=true to "
					  "enable) Would have closed: "
					;
			auto first = true;
			for (auto const& c : closemap) {
				/* Filter out unmanaged nodes.  */
				if (unmanaged.find(c.first) != unmanaged.end())
					continue;

				closes->emplace_back(c);
				/* Build message.  */
				if (first)
					first = false;
				else
					message += "; ";
				message += std::string(c.first);
			}

			/* Nothing to close after filtering?  */
			if (closes->empty())
				return Ev::lift();

			return Boss::log( bus, Info
					, "PeerJudge: Executioner: %s"
					, message.c_str()
					);
		}).then([this, closes]() {
			/* If not enabled or nothing to close,
			 * do nothing.
			 */
			if (!enabled || closes->empty())
				return Ev::lift();

			/* Execute.  */
			auto execute = [this
				       ](std::pair< Ln::NodeId
						  , std::string
						  > item) {
				auto id = std::string(item.first);
				auto reason = item.second;
				return Ev::lift().then([this]() {
					
					/* Record in db.  */
					return db.transact();
				}).then([this, id, reason](Sqlite3::Tx tx) {

					tx.query(R"QRY(
					INSERT INTO "PeerJudge_executed"
					 VALUES(:time, :node, :reason);
					)QRY")
						.bind(":time", Ev::now())
						.bind( ":node"
						     , id
						     )
						.bind( ":reason"
						     , reason
						     )
						.execute()
						;
					tx.commit();

					/*Perform the actual close.  */
					auto parms = Json::Out()
						.start_object()
							.field("id" , id)
							.field( "unilateraltimeout"
							      , channel_close_timeout
							      )
							.field( "fee_negotiation_step"
							      , fee_negotiation_step
							      )
						.end_object();
					return rpc->command( "close"
							   , std::move(parms)
							   );
				}).then([](Jsmn::Object _) {
					return Ev::lift();
				}).catching<RpcError>([this, id](RpcError e) {
					auto err = Util::stringify(e.error);
					return Boss::log( bus, Error
							, "PeerJudge: "
							  "close %s "
							  "error: %s"
							, id.c_str()
							, err.c_str()
							);
				});
			};

			auto action = Ev::foreach( std::move(execute)
						 , std::move(*closes)
						 );
			/* Do the closing in the background.  */
			return Boss::concurrent(action);
		});
	});
}
Ev::Io<void> Executioner::wait_init() {
	return Ev::yield().then([this]() {
		if (db && rpc)
			return Ev::lift();
		return wait_init();
	});
}

}}}
