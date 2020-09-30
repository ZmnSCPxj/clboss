#include"Boss/Mod/PeerCompetitorFeeMonitor/Main.hpp"
#include"Boss/Mod/PeerCompetitorFeeMonitor/Surveyor.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ListpeersAnalyzedResult.hpp"
#include"Boss/Msg/PeerMedianChannelFee.hpp"
#include"Boss/Msg/TimerRandomHourly.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/foreach.hpp"
#include"Ev/map.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<algorithm>
#include<iterator>
#include<sstream>

namespace Boss { namespace Mod { namespace PeerCompetitorFeeMonitor {

class Main::Impl {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;

	Ln::NodeId self_id;

	/* Current known peers with channels.  */
	std::vector<Ln::NodeId> channels;
	/* Whether the above vector is valid.  */
	bool have_channels;
	/* Whether we have fired the initial channel fees.  */
	bool fired_init;

public:
	Impl(S::Bus& bus_
	    ) : bus(bus_)
	      , rpc(nullptr)
	      , have_channels(false)
	      { start(); }
private:
	void start() {
		bus.subscribe<Msg::Init
			     >([this](Msg::Init const& init) {
			rpc = &init.rpc;
			self_id = init.self_id;
			if (have_channels && !fired_init) {
				fired_init = true;
				return Boss::concurrent(on_periodic());
			}
			return Ev::lift();
		});
		bus.subscribe<Msg::ListpeersAnalyzedResult
			     >([this](Msg::ListpeersAnalyzedResult const& r) {
			/* Copy into channels vector.  */
			channels.clear();
			std::copy( r.connected_channeled.begin()
				 , r.connected_channeled.end()
				 , std::back_inserter(channels)
				 );
			std::copy( r.disconnected_channeled.begin()
				 , r.disconnected_channeled.end()
				 , std::back_inserter(channels)
				 );
			have_channels = true;

			if (rpc && !fired_init) {
				fired_init = true;
				return Boss::concurrent(on_periodic());
			}
			return Ev::lift();
		});
		bus.subscribe<Msg::TimerRandomHourly
			     >([this](Msg::TimerRandomHourly const&) {
			if (!rpc || !have_channels)
				return Ev::lift();
			return on_periodic();
		});
	}

	Ev::Io<void> on_periodic() {
		return Ev::lift().then([this]() {
			auto f = [this](Ln::NodeId nid) {
				auto surveyor = Surveyor::create
						( bus
						, *rpc
						, self_id
						, std::move(nid)
						);
				return surveyor->run();
			};
			/* Do not std::move channels --- we need to retain
			 * it.  */
			return Ev::map(f, channels);
		}).then([this](std::vector<std::unique_ptr<Surveyor::Result>> results) {
			/* Build report.  */
			auto os = std::ostringstream();
			auto first = true;
			for (auto const& r : results) {
				if (!r)
					continue;
				if (first)
					first = false;
				else
					os << ", ";
				os << r->peer_id << "("
				   << "b = " << r->median_base << ", "
				   << "p = " << r->median_proportional
				   << ")"
				    ;
			}
			auto act = Boss::log( bus, Debug
					    , "PeerCompetitorFeeMonitor: "
					      "Weighted median fees: %s"
					    , os.str().c_str()
					    );

			auto f = [this](std::unique_ptr<Surveyor::Result> r) {
				if (!r)
					return Ev::lift();
				return bus.raise(Msg::PeerMedianChannelFee{
					std::move(r->peer_id),
					r->median_base,
					r->median_proportional
				});
			};
			act += Ev::foreach(f, std::move(results));

			return act;
		});
	}
};

Main::Main(Main&&) =default;
Main& Main::operator=(Main&&) =default;
Main::~Main() =default;

Main::Main(S::Bus& bus) : pimpl(Util::make_unique<Impl>(bus)) { }

}}}
