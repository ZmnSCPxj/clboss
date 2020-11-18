#include"Boss/Mod/ChannelFundsComputer.hpp"
#include"Boss/Msg/ChannelFunds.hpp"
#include"Boss/Msg/ListfundsResult.hpp"
#include"Jsmn/Object.hpp"
#include"Ln/Amount.hpp"
#include"S/Bus.hpp"

namespace Boss { namespace Mod {

void ChannelFundsComputer::start() {
	typedef ChannelFundsComputer This;
	using std::placeholders::_1;
	bus.subscribe< Msg::ListfundsResult
		     >(std::bind(&This::on_listfunds, this, _1));
}
Ev::Io<void>
ChannelFundsComputer::on_listfunds(Msg::ListfundsResult const& r) {
	return Ev::lift().then([this, r]() {
		/* Stack em sats.  */
		auto total = Ln::Amount::sat(0);
		auto connected = Ln::Amount::sat(0);
		for (auto chan : r.channels) {
			if (!chan.is_object() || !chan.has("our_amount_msat"))
				continue;
			auto amount_j = chan["our_amount_msat"];
			if (!amount_j.is_string())
				continue;
			auto amount_s = std::string(amount_j);
			if (!Ln::Amount::valid_string(amount_s))
				continue;
			auto amount = Ln::Amount(amount_s);
			total += amount;

			if (!chan.has("connected"))
				continue;
			auto is_connected = chan["connected"];
			if (!is_connected.is_boolean())
				continue;
			if (!((bool) is_connected))
				continue;
			connected += amount;
		}

		return bus.raise(Msg::ChannelFunds{total, connected});
	});
}

}}
