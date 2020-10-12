#include"Boss/Mod/StatusCommand.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/ManifestCommand.hpp"
#include"Boss/Msg/ProvideStatus.hpp"
#include"Boss/Msg/SolicitStatus.hpp"
#include"Boss/concurrent.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"S/Bus.hpp"

namespace Boss { namespace Mod {

void StatusCommand::start() {
	using std::placeholders::_1;
	typedef StatusCommand This;

	bus.subscribe<Msg::Manifestation>(
		std::bind(&This::on_manifest, this, _1)
	);
	bus.subscribe<Msg::CommandRequest>(
		std::bind(&This::on_command, this, _1)
	);
	bus.subscribe<Msg::ProvideStatus>(
		std::bind(&This::on_status, this, _1)
	);
}
Ev::Io<void> StatusCommand::on_manifest(Boss::Msg::Manifestation const&) {
	return bus.raise(Msg::ManifestCommand{
		"clboss-status", "", "Get CLBOSS status.",
		false
	});
}
Ev::Io<void> StatusCommand::on_command(Boss::Msg::CommandRequest const& c) {
	if (c.command != "clboss-status")
		return Ev::lift();

	/* Could happen as a race condition.
	 * Just busy-spin.
	 */
	if (soliciting) {
		return Ev::yield().then([this, c]() {
			return Boss::concurrent(Ev::yield().then([this, c]() {
				return on_command(c);
			}));
		});
	}

	soliciting = true;
	id = c.id;
	return Ev::yield().then([this]() {
		return bus.raise(Msg::SolicitStatus{});
	}).then([this]() {
		soliciting = false;
		auto fields = std::move(this->fields);
		auto result = Json::Out();
		auto obj = result.start_object();
		for (auto& f : fields)
			obj.field(f.first, std::move(f.second));
		obj.end_object();
		return bus.raise(Msg::CommandResponse{id, result});
	});
}
Ev::Io<void> StatusCommand::on_status(Boss::Msg::ProvideStatus const& s) {
	if (!soliciting)
		return Ev::lift();
	fields[s.key] = s.value;
	return Ev::lift();
}

}}
