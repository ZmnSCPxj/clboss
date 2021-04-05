#include"Boss/Mod/ChannelCreationDecider.hpp"
#include"Boss/Msg/ChannelFunds.hpp"
#include"Boss/Msg/ManifestOption.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/NeedsOnchainFunds.hpp"
#include"Boss/Msg/OnchainFee.hpp"
#include"Boss/Msg/OnchainFunds.hpp"
#include"Boss/Msg/Option.hpp"
#include"Boss/Msg/RequestChannelCreation.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"
#include<sstream>

namespace {

/* How much onchain funds to reserve for anchor commitments.  */
auto const default_reserve = Ln::Amount::sat(30000);
/* How much onchain funds to add when emitting NeedsOnchainFunds.  */
auto const needs_reserve = Ln::Amount::sat(30000);
/* How much minimum funds to put in channels.  */
auto const min_amount = Ln::Amount::btc(0.010);
/* How much we are willing to emit as "needs onchain".  */
auto const max_needs_onchain = Ln::Amount::btc(0.010);

/* If our onchain funds are below this % of our total, hold off on
 * creating channels.
 */
auto const onchain_percent_min = double(10);
/* If high fees, if onchain funds is more than this percent of
 * total funds, create channels.
 * If below this, wait for low-fee region.
 */
auto const onchain_percent_max = double(25);

/* If onchain funds are above or equal to this amount, but in
 * onchain-percent terms is below onchain_percent_min, create
 * channels anyway --- the channels are going to be fairly
 * large still, so okay even if below onchain_percent_min.
 * This only comes into play for really rich nodes with
 * 1.6777215 BTC or more.
 */
auto const max_onchain_holdoff = Ln::Amount::sat(16777215);

}

namespace Boss { namespace Mod {

class ChannelCreationDecider::Impl {
private:
	S::Bus& bus;

	std::unique_ptr<bool> low_fee;
	std::unique_ptr<Ln::Amount> channels;
	std::unique_ptr<Ln::Amount> saved_onchain;
	Ln::Amount reserve;

	void start() {
		reserve = default_reserve;

		bus.subscribe< Msg::ChannelFunds
			     >([this](Msg::ChannelFunds const& m) {
			if (!channels)
				channels = Util::make_unique<Ln::Amount>();
			*channels = m.total;
			return run();
		});
		bus.subscribe< Msg::OnchainFee
			     >([this](Msg::OnchainFee const& m) {
			if (!low_fee)
				low_fee = Util::make_unique<bool>();
			*low_fee = m.fees_low;
			return run();
		});
		bus.subscribe< Msg::OnchainFunds
			     >([this](Msg::OnchainFunds const& m) {
			if (!saved_onchain)
				saved_onchain = Util::make_unique<Ln::Amount>();
			*saved_onchain = m.onchain;
			return run();
		});

		/* Option --clboss-min-onchain.  */
		bus.subscribe< Msg::Manifestation
			     >([this](Msg::Manifestation const& _) {
			return bus.raise(Msg::ManifestOption{
				"clboss-min-onchain", Msg::OptionType_String,
				Json::Out::direct(default_reserve.to_sat()),
				"Target to leave this number of satoshis "
				"onchain, putting the rest into channels."
			});
		});
		bus.subscribe< Msg::Option
			     >([this](Msg::Option const& o) {
			if (o.name != "clboss-min-onchain")
				return Ev::lift();
			auto is = std::istringstream(std::string(o.value));
			auto sats = std::uint64_t();
			is >> sats;
			reserve = Ln::Amount::sat(sats);

			/* If same as default value, do not spam logs.  */
			if (reserve == default_reserve)
				return Ev::lift();

			return Boss::log( bus, Info
					, "ChannelCreationDecider: "
					  "Onchain reserve set by "
					  "--clboss-min-onchain to "
					  "%s satoshis."
					, std::string(o.value).c_str()
					);
		});
	}

	Ev::Io<void> run() {
		/* Do nothing if not complete information yet.  */
		if (!(low_fee && channels && saved_onchain))
			return Ev::lift();

		/* Get this trigger.  */
		auto onchain = std::move(saved_onchain);

		/* How much total money do we have anyway?  */
		auto total = *channels + *onchain;

		/* Is the amount the reserve?  */
		if (*onchain <= reserve)
			/* Do nothing and be silent.  */
			return decide_do_nothing_silently();

		/* What is our minimum channeling amount?  */
		auto target = total * (onchain_percent_min / 100.0);
		if (target < min_amount)
			target = min_amount;
		if (target > max_onchain_holdoff)
			target = max_onchain_holdoff;
		/* Make sure to add the reserve!  */
		target += reserve;
		/* Below our minimum channeling amount?  */
		if (*onchain < target) {
			/* Add the needs_reserve here because fees.  */
			auto more = ( target
				    + needs_reserve
				    )
				  - *onchain
				  ;
			auto more_btc = more.to_btc();
			/* Totally not a "just send me more funds to
			 * redeem your money" scam.  */
			return decide( std::string("Onchain amount too low, ")
				     + "add " + Util::stringify(more_btc)
				     + " or more before we create channels"
				     , nullptr
				     ).then([this, more]() {
				if (more > max_needs_onchain) {
					return Boss::log( bus, Debug
							, "ChannelCreationDecider: "
							  "We need %s, which is "
							  "greater than our limit of "
							  "%s; will NOT move funds "
							  "from channels to onchain."
							, std::string(more).c_str()
							, std::string(max_needs_onchain)
								.c_str()
							);
				}
				/* And tell everyone about the
				 * missingfunds.  */
				return bus.raise(Msg::NeedsOnchainFunds{
					more
				});
			});
		}

		if (*low_fee)
			return decide("Low fees now", std::move(onchain));

		auto onchain_percent = (*onchain / total) * 100;
		if (onchain_percent > onchain_percent_max)
			return decide( std::string("High fees, but ")
				     + Util::stringify(onchain_percent)
				     + "% of our funds are onchain, "
				       "which is above our limit of "
				     + Util::stringify(onchain_percent_max)
				     + "%"
				     , std::move(onchain)
				     );

		return decide("High fees now", nullptr);
	}

	Ev::Io<void> decide( std::string comment
			   , std::unique_ptr<Ln::Amount> pfunding
			   ) {
		if (!pfunding)
			return Boss::log( bus, Info
					, "ChannelCreationDecider: %s. "
					  "Will not create channels yet."
					, comment.c_str()
					);
		auto funding = *pfunding;
		auto actual = funding - reserve;
		return Boss::concurrent( bus.raise(Msg::RequestChannelCreation{actual})
				       ).then([ this
					      , actual
					      , comment
					      ]() {
			return Boss::log( bus, Info
					, "ChannelCreationDecider: %s."
					  "Will create channels worth %s "
					  "(%s reserved for onchain actions)."
					, comment.c_str()
					, std::string(actual).c_str()
					, std::string(reserve).c_str()
					);
		});
	}
	Ev::Io<void> decide_do_nothing_silently() {
		return Ev::lift();
	}

public:
	Impl() =delete;
	Impl(Impl const&) =delete;

	Impl(S::Bus& bus_) : bus(bus_) { start(); }
};

ChannelCreationDecider::ChannelCreationDecider(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }
ChannelCreationDecider::ChannelCreationDecider(ChannelCreationDecider&& o)
	: pimpl(std::move(o.pimpl))  { }
ChannelCreationDecider::~ChannelCreationDecider() { }

}}
