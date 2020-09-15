#include"Boss/Mod/ChannelCreationDecider.hpp"
#include"Boss/Msg/ChannelFunds.hpp"
#include"Boss/Msg/OnchainFee.hpp"
#include"Boss/Msg/OnchainFunds.hpp"
#include"Boss/Msg/RequestChannelCreation.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"

namespace {

/* How much onchain funds to reserve for anchor commitments.  */
auto const reserve = Ln::Amount::sat(30000);
/* How much minimum funds to put in channels.  */
auto const min_amount = Ln::Amount::btc(0.010);

/* If high fees, if onchain funds is more than this percent of
 * total funds, create channels.
 * If below this, wait for low-fee region.
 */
auto const onchain_percent_max = double(25);

}

namespace Boss { namespace Mod {

class ChannelCreationDecider::Impl {
private:
	S::Bus& bus;

	std::unique_ptr<bool> low_fee;
	std::unique_ptr<Ln::Amount> channels;
	std::unique_ptr<Ln::Amount> saved_onchain;

	void start() {
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
	}

	Ev::Io<void> run() {
		/* Do nothing if not complete information yet.  */
		if (!(low_fee && channels && saved_onchain))
			return Ev::lift();

		/* Get this trigger.  */
		auto onchain = std::move(saved_onchain);

		/* Below our minimum channeling amount?  */
		if (*onchain < min_amount + reserve) {
			/* Double up the reserve here because fees.  */
			auto more = (min_amount + reserve * 2) - *onchain;
			return decide( std::string("Onchain amount too low, ")
				     + "add " + std::string(more) + " or more"
				     , nullptr
				     );
		}

		if (*low_fee)
			return decide("Low fees now", std::move(onchain));

		auto total = *channels + *onchain;
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
