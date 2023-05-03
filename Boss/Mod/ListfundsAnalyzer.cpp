#include"Boss/Mod/ListfundsAnalyzer.hpp"
#include"Boss/Msg/ListfundsAnalyzedResult.hpp"
#include"Boss/Msg/ListfundsResult.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"

namespace Boss { namespace Mod {

class ListfundsAnalyzer::Impl {
private:
	S::Bus& bus;

	void start() {
		bus.subscribe<Msg::ListfundsResult
			     >([this](Msg::ListfundsResult const& l) {
			try {
				return analyze(l);
			} catch (Jsmn::TypeError const&) {
				return Boss::log( bus, Error
						, "ListfundsAnalyzer: Unexpected "
						  "result from listfunds: "
						  "outputs = %s, "
						  "channels = %s"
						, Util::stringify(l.outputs)
							.c_str()
						, Util::stringify(l.channels)
							.c_str()
						);
			}
		});
	}

	Ev::Io<void> analyze(Msg::ListfundsResult const& l) {
		auto ar = Msg::ListfundsAnalyzedResult();

		ar.total_owned = Ln::Amount::msat(0);

		for (auto output : l.outputs) {
			auto amount_msat = output["amount_msat"];
			if (!Ln::Amount::valid_object(amount_msat))
				throw Jsmn::TypeError();
			ar.total_owned +=
				Ln::Amount::object(amount_msat);
		}

		for (auto channel : l.channels) {
			auto amount_msat = channel["our_amount_msat"];
			if (!Ln::Amount::valid_object(amount_msat))
				throw Jsmn::TypeError();
			ar.total_owned +=
				Ln::Amount::object(amount_msat);
		}

		return bus.raise(std::move(ar));
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;

	explicit
	Impl(S::Bus& bus_) : bus(bus_) { start(); }
};

ListfundsAnalyzer::ListfundsAnalyzer(ListfundsAnalyzer&&) =default;
ListfundsAnalyzer::~ListfundsAnalyzer() =default;
ListfundsAnalyzer::ListfundsAnalyzer(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
