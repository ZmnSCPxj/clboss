#include"Boss/Mod/ComplainerByLowConnectRate.hpp"
#include"Boss/Msg/RaisePeerComplaint.hpp"
#include"Boss/Msg/SolicitPeerComplaints.hpp"
#include"Ev/Io.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"

namespace {

/* If `connect_rate` is lower than this in the 3-day metrics,
 * complain.  */
auto constexpr min_acceptable_connect_rate = double(0.25);

}

namespace Boss { namespace Mod {

class ComplainerByLowConnectRate::Impl {
private:
	S::Bus& bus;

	void start() {
		bus.subscribe<Msg::SolicitPeerComplaints
			     >([this](Msg::SolicitPeerComplaints const& metrics) {
			return Ev::lift().then([this, metrics]() {
				auto act = Ev::lift();

				for (auto const& m : metrics.day3) {
					if (!m.second.connect_rate)
						continue;
					auto connect_rate = *m.second.connect_rate;
					if (connect_rate >= min_acceptable_connect_rate)
						continue;

					auto reason = std::string()
						    + "ComplainerByLowConnectRate: "
						    + "Connect rate in past three days "
						    + "is only "
						    + Util::stringify(connect_rate) + ", "
						    + "minimum is "
						    + Util::stringify(
							min_acceptable_connect_rate
						      )
						    ;

					act += bus.raise(Msg::RaisePeerComplaint{
						m.first,
						std::move(reason)
					});
				}

				return act;
			});
		});
	}

public:
	explicit
	Impl( S::Bus& bus_
	    ) : bus(bus_)
	      { start(); }
};

ComplainerByLowConnectRate::ComplainerByLowConnectRate(ComplainerByLowConnectRate&&) =default;
ComplainerByLowConnectRate::~ComplainerByLowConnectRate() =default;
ComplainerByLowConnectRate::ComplainerByLowConnectRate(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
