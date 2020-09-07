#include"Boss/Mod/CommandReceiver.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/JsonCin.hpp"
#include"Boss/Msg/JsonCout.hpp"
#include"Boss/Msg/Notification.hpp"
#include"S/Bus.hpp"

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
			return bus.raise(Boss::Msg::Notification{
				method, params
			});
		} else {
			if (!inp["id"].is_number())
				return Ev::lift();

			/* Command.  */
			auto id = std::uint64_t((double) inp["id"]);
			pendings.insert(id);

			return bus.raise(Boss::Msg::CommandRequest{
				method, params, id
			});
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
				.field("id", double(resp.id))
				.field("result", resp.response)
			.end_object()
			;
		return bus.raise(Boss::Msg::JsonCout{std::move(js)});
	});
}

}}
