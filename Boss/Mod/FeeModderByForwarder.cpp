#include"Boss/Mod/FeeModderByForwarder.hpp"
#include"Boss/Msg/ProvideChannelFeeModifierDetailed.hpp"
#include"Boss/Msg/SolicitChannelFeeModifier.hpp"
#include"Ev/Io.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"

namespace {

/* Increase our base fee, reduce our prop fee.  */
auto constexpr base_mult = double(2.0);
auto constexpr prop_mult = double(0.5);

}

namespace Boss { namespace Mod {

class FeeModderByForwarder::Impl {
public:
	Impl()=delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;
	explicit
	Impl(S::Bus& bus) {
		bus.subscribe< Msg::SolicitChannelFeeModifier
			     >([&bus](Msg::SolicitChannelFeeModifier const& _) {
			return bus.raise(Msg::ProvideChannelFeeModifierDetailed{
				[](Ln::NodeId, std::uint32_t, std::uint32_t) {
					return Ev::lift(std::make_pair(
						base_mult, prop_mult
					));
				}
			});
		});
	}
};


FeeModderByForwarder::FeeModderByForwarder(FeeModderByForwarder&&) =default;
FeeModderByForwarder::~FeeModderByForwarder() =default;
FeeModderByForwarder::FeeModderByForwarder(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
