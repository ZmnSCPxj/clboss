#include"Boss/Mod/FeeModderByExpenditures.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/ProvideChannelFeeModifier.hpp"
#include"Boss/Msg/RequestEarningsInfo.hpp"
#include"Boss/Msg/ResponseEarningsInfo.hpp"
#include"Boss/Msg/SolicitChannelFeeModifier.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"
#include<math.h>
#include<string>

namespace {

/* Base to use for logarithm.  */
auto constexpr log_base = double(10);
/* Maximum multiplier.  */
auto constexpr max_multiplier = double(5.0);

}

namespace Boss { namespace Mod {

class FeeModderByExpenditures::Impl {
private:
	S::Bus& bus;
	Boss::ModG::ReqResp< Msg::RequestEarningsInfo
			   , Msg::ResponseEarningsInfo
			   > earnings_rr;

	void start() {
		bus.subscribe< Msg::SolicitChannelFeeModifier
			     >([this](Msg::SolicitChannelFeeModifier const& _) {
			return bus.raise(Msg::ProvideChannelFeeModifier{
				[this](Ln::NodeId n, std::uint32_t _, std::uint32_t __) {
					return modify_fee(n);
				}
			});
		});
	}

	Ev::Io<double>
	modify_fee(Ln::NodeId const& n) {
		return Ev::lift().then([this, n]() {
			return earnings_rr.execute(Msg::RequestEarningsInfo{
				nullptr, n
			});
		}).then([this, n](Msg::ResponseEarningsInfo e) {
			auto msg = std::string();
			auto factor = double();
			decide_fee_factor(msg, factor, e.out_earnings, e.out_expenditures);

			return Boss::log( bus, Debug
					, "FeeModderByExpenditures: %s "
					  "(earn = %s, spend = %s): %s"
					, Util::stringify(n).c_str()
					, Util::stringify(e.out_earnings).c_str()
					, Util::stringify(e.out_expenditures).c_str()
					, msg.c_str()
					).then([factor]() {
				return Ev::lift(factor);
			});
		});
	}

	void decide_fee_factor( std::string& msg
			      , double& factor
			      , Ln::Amount earnings
			      , Ln::Amount expenditures
			      ) {
		if (earnings >= expenditures) {
			msg = "Channel not at a loss, factor = 1.";
			factor = 1.0;
			return;
		}

		/* Avoid divide by 0.  */
		if (earnings == Ln::Amount::msat(0)) {
			msg = std::string("No earnings, factor = max = ")
			    + Util::stringify(int(max_multiplier))
			    + "."
			    ;
			factor = max_multiplier;
			return;
		}

		auto ratio = expenditures / earnings;
		factor = 1.0 + round(::log(ratio) / ::log(log_base));
		msg = std::string("spend / earn = ")
		    + Util::stringify(ratio)
		    + ", factor = "
		    ;
		if (factor <= max_multiplier)
			msg += Util::stringify(int(factor))
			     + "."
			     ;
		else {
			factor = max_multiplier;
			msg += std::string("max = ")
			     + Util::stringify(int(factor))
			     + "."
			     ;
		}
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	explicit
	Impl( S::Bus& bus_
	    ) : bus(bus_)
	      , earnings_rr( bus_
			   , [](Msg::RequestEarningsInfo& m, void* p) {
				m.requester = p;
			     }
			   , [](Msg::ResponseEarningsInfo& m) {
				return m.requester;
			     }
			   )
	      { start(); }
};

FeeModderByExpenditures::FeeModderByExpenditures(FeeModderByExpenditures&&) =default;
FeeModderByExpenditures::~FeeModderByExpenditures() =default;

FeeModderByExpenditures::FeeModderByExpenditures(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
