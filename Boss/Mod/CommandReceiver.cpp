#include"Boss/Mod/CommandReceiver.hpp"
#include"Boss/Msg/CommandFail.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/JsonCin.hpp"
#include"Boss/Msg/JsonCout.hpp"
#include"Boss/Msg/ManifestHook.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/Notification.hpp"
#include"Boss/Msg/RpcCommandHook.hpp"
#include"Boss/concurrent.hpp"
#include"Ln/CommandId.hpp"
#include"S/Bus.hpp"
#include<sstream>

namespace Boss { namespace Mod {

CommandReceiver::CommandReceiver(S::Bus& bus_) : bus(bus_) {
	bus.subscribe<Boss::Msg::JsonCin>([this](Boss::Msg::JsonCin const& cin_msg) {
		auto& inp = cin_msg.obj;

		/* Silently fail.  */
		if (!inp.is_object())
			return Ev::lift();
		if (!inp.has("method"))
			return Ev::lift();
		if (!inp["method"].is_string())
			return Ev::lift();
		if (!inp.has("params"))
			return Ev::lift();

		auto method = std::string(inp["method"]);
		auto params = inp["params"];

		if (!inp.has("id")) {
			/* Notification.  */
			return Boss::concurrent(
				bus.raise(Boss::Msg::Notification{
					method, params
				})
			);
		} else if (method == "rpc_command") {
			/* Respond immediately!  */
			auto js = Json::Out()
				.start_object()
					.field("jsonrpc", std::string("2.0"))
					.field("id", inp["id"])
					.start_object("result")
						.field("result", "continue")
					.end_object()
				.end_object()
				;
			return bus.raise(Msg::JsonCout{std::move(js)})
			     /* Execute in background.  */
			     + Boss::concurrent(
					bus.raise(Msg::RpcCommandHook{params})
			       )
			     ;
		} else {
			auto pid = Ln::command_id_from_jsmn_object(
				inp["id"]
			);
			if (!pid)
				return Ev::lift();

			/* Command.  */
			auto const& id = *pid;
			pendings.insert(id);

			return Boss::concurrent(
				bus.raise(Boss::Msg::CommandRequest{
					method, params, id
				})
			);
		}
	});
	bus.subscribe<Boss::Msg::CommandResponse>([this](Boss::Msg::CommandResponse const& resp) {
		/* If not a pending command, ignore.  */
		auto it = pendings.find(resp.id);
		if (it == pendings.end())
			return Ev::lift();
		pendings.erase(it);
		/* Construct the JSON result.  */
		auto js = Json::Out()
			.start_object()
				.field("jsonrpc", std::string("2.0"))
				.field("id", resp.id)
				.field("result", resp.response)
			.end_object()
			;
		return bus.raise(Boss::Msg::JsonCout{std::move(js)});
	});
	bus.subscribe<Boss::Msg::CommandFail>([this](Boss::Msg::CommandFail const& fail) {
		/* If not a pending command, ignore.  */
		auto it = pendings.find(fail.id);
		if (it == pendings.end())
			return Ev::lift();
		pendings.erase(it);
		/* Construct the JSON result.  */
		auto js = Json::Out()
			.start_object()
				.field("jsonrpc", std::string("2.0"))
				.field("id", fail.id)
				.start_object("error")
					.field("code", fail.code)
					.field("message", fail.message)
					.field("data", fail.data)
				.end_object()
			.end_object()
			;
		return bus.raise(Boss::Msg::JsonCout{std::move(js)});
	});
	bus.subscribe<Msg::Manifestation
		     >([this](Msg::Manifestation const& _) {
		return bus.raise(Msg::ManifestHook{"rpc_command"});
	});
}

}}
