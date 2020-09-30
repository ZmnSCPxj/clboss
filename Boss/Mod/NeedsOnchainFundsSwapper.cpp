#include"Boss/Mod/NeedsOnchainFundsSwapper.hpp"
#include"Boss/ModG/Swapper.hpp"
#include"Boss/Msg/NeedsOnchainFunds.hpp"
#include"Boss/Msg/OnchainFee.hpp"
#include"Ev/Io.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<sstream>

namespace Boss { namespace Mod {

class NeedsOnchainFundsSwapper::Impl : ModG::Swapper {
public:
	Impl(S::Bus& bus_) : ModG::Swapper( bus_
					  , "NeedsOnchainFundsSwapper"
					  , "needs_onchain_funds_swapper"
					  ) { start(); }
private:
	std::unique_ptr<bool> fees_low;

	void start() {
		bus.subscribe<Msg::OnchainFee
			     >([this](Msg::OnchainFee const& m) {
			if (!fees_low)
				fees_low = Util::make_unique<bool>();
			*fees_low = m.fees_low;
			return Ev::lift();
		});
		bus.subscribe<Msg::NeedsOnchainFunds
			     >([this](Msg::NeedsOnchainFunds const& m) {
			if (!fees_low)
				return Ev::lift();
			auto needed = m.needed;
			auto f = [ this
				 , needed
				 ]( Ln::Amount& amount
				  , std::string& message
				  ) {
				if (!*fees_low) {
					message = "Onchain fees high";
					return false;
				}

				amount = needed * 1.12;

				auto os = std::ostringstream();
				os << "Need " << needed
				   << " onchain"
				    ;
				message = os.str();

				return true;
			};
			return trigger_swap(f);
		});
	}
};

NeedsOnchainFundsSwapper::NeedsOnchainFundsSwapper(NeedsOnchainFundsSwapper&&)
	=default;
NeedsOnchainFundsSwapper&
NeedsOnchainFundsSwapper::operator=(NeedsOnchainFundsSwapper&&) =default;
NeedsOnchainFundsSwapper::~NeedsOnchainFundsSwapper() =default;


NeedsOnchainFundsSwapper::NeedsOnchainFundsSwapper(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
