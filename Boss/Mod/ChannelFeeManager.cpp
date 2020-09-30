#include"Boss/Mod/ChannelFeeManager.hpp"
#include"Boss/Msg/PeerMedianChannelFee.hpp"
#include"Boss/Msg/ProvideChannelFeeModifier.hpp"
#include"Boss/Msg/SetChannelFee.hpp"
#include"Boss/Msg/SolicitChannelFeeModifier.hpp"
#include"Boss/concurrent.hpp"
#include"Ev/Io.hpp"
#include"Ev/map.hpp"
#include"Ev/yield.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include<math.h>
#include<memory>

namespace Boss { namespace Mod {

void ChannelFeeManager::start() {
	bus.subscribe<Msg::PeerMedianChannelFee
		     >([this](Msg::PeerMedianChannelFee const& m) {
		return Boss::concurrent(
			perform(m.node, m.base, m.proportional)
		);
	});
	bus.subscribe<Msg::ProvideChannelFeeModifier
		     >([this](Msg::ProvideChannelFeeModifier const& m) {
		if (soliciting)
			modifiers.push_back(m.modifier);
		return Ev::lift();
	});
}
Ev::Io<void> ChannelFeeManager::solicit() {
	/* Busy wait.  */
	if (soliciting && !initted)
		return Ev::yield().then([this]() { return solicit(); });

	if (initted)
		return Ev::lift();

	soliciting = true;
	return bus.raise( Msg::SolicitChannelFeeModifier{}
			).then([this]() {
		soliciting = false;
		initted = true;
		return Ev::lift();
	});
}
Ev::Io<void>
ChannelFeeManager::perform( Ln::NodeId node
			  , std::uint32_t base
			  , std::uint32_t proportional
			  ) {
	/* This message will be mutated later.  */
	auto msg = std::make_shared<Msg::SetChannelFee>();
	msg->node = node;
	msg->base = base;
	msg->proportional = proportional;
	return Ev::lift().then([this]() {
		return solicit();
	}).then([this, msg]() {
		/* Run the modifiers */
		auto f = [msg](std::function<Ev::Io<double>( Ln::NodeId
							   , std::uint32_t
							   , std::uint32_t
							   )
					    > modifier) {
			return modifier( msg->node
				       , msg->base
				       , msg->proportional
				       );
		};
		return Ev::map(std::move(f), modifiers);
	}).then([this, msg](std::vector<double> multipliers) {
		/* Get double versions of the base and proportional fees.  */
		auto b = double(msg->base);
		auto p = double(msg->proportional);
		/* Multiply by all, using the `double` versions.  */
		for (auto m : multipliers) {
			b *= m;
			p *= m;
		}
		/* Round and convert back to uint32_t.  */
		msg->base = std::uint32_t(round(b));
		msg->proportional = std::uint32_t(round(p));
		/* If 0, set to 1.  */
		if (msg->base == 0)
			msg->base = 1;
		if (msg->proportional == 0)
			msg->proportional = 1;

		/* Send the message!  */
		return bus.raise(std::move(*msg));
	});
}

}}
