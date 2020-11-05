#include"Boss/Mod/PeerMetrician.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/RequestPeerMetrics.hpp"
#include"Boss/Msg/RequestPeerStatistics.hpp"
#include"Boss/Msg/ResponsePeerMetrics.hpp"
#include"Boss/Msg/ResponsePeerStatistics.hpp"
#include"Boss/Msg/ProvideStatus.hpp"
#include"Boss/Msg/SolicitStatus.hpp"
#include"Ev/Io.hpp"
#include"Ev/map.hpp"
#include"Ev/now.hpp"
#include"Util/make_unique.hpp"
#include<algorithm>
#include<iterator>

namespace Boss { namespace Mod {

class PeerMetrician::Impl {
private:
	S::Bus& bus;
	ModG::ReqResp< Msg::RequestPeerStatistics
		     , Msg::ResponsePeerStatistics
		     > statistician;

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	Impl(S::Bus& bus_
	    ) : bus(bus_)
	      , statistician( bus_
			    , [](Msg::RequestPeerStatistics& m, void* p) {
					m.requester = p;
			      }
			    , [](Msg::ResponsePeerStatistics& m) {
					return m.requester;
			      }
			    )
	      {
		start();
	}

private:
	void start() {
		bus.subscribe<Msg::RequestPeerMetrics
			     >([this](Msg::RequestPeerMetrics const& m) {
			return run(m);
		});
		bus.subscribe<Msg::SolicitStatus
			     >([this](Msg::SolicitStatus const& m) {
			return Ev::lift().then([this]() {
				return get_metrics();
			}).then([this](Msg::ResponsePeerMetrics m) {
				return bus.raise(Msg::ProvideStatus{
					"peer_metrics", provide_metrics(m)
				});
			});
		});
	}
	Ev::Io<std::map<Ln::NodeId, Msg::PeerStatistics>>
	get_stats(double start, double end) {
		return statistician.execute(Msg::RequestPeerStatistics{
			nullptr, start, end
		}).then([](Msg::ResponsePeerStatistics resp) {
			return Ev::lift(std::move(resp.statistics));
		});
	}
	static
	std::map<Ln::NodeId, Msg::PeerMetrics>
	map_stats_to_mets(std::map< Ln::NodeId
				  , Msg::PeerStatistics
				  > const& stats) {
		auto mets = std::map<Ln::NodeId, Msg::PeerMetrics>();
		std::transform( stats.begin(), stats.end()
			      , std::inserter(mets, mets.begin())
			      , [](std::pair< Ln::NodeId
					    , Msg::PeerStatistics
					    > const& entry) {
			auto mets = stats_to_mets(entry.second);
			return std::make_pair(entry.first, std::move(mets));
		});
		return mets;
	}
	static
	Msg::PeerMetrics
	stats_to_mets(Msg::PeerStatistics const& stats) {
		auto mets = Msg::PeerMetrics();
		auto time = stats.end_time - stats.start_time;
		mets.age = stats.age;
		if (stats.attempts > 0) {
			mets.seconds_per_attempt = stats.lockrealtime
						 / double(stats.attempts)
						 ;
			mets.success_per_attempt = std::make_shared<double>();
			*mets.success_per_attempt = double(stats.successes)
						  / double(stats.attempts)
						  ;
		} else {
			mets.seconds_per_attempt = 0;
		}
		mets.success_per_day = (double(stats.successes) * 86400)
				     / time
				     ;
		if (stats.connect_checks > 0) {
			mets.connect_rate = std::make_shared<double>();
			*mets.connect_rate = double(stats.connects)
					   / double(stats.connect_checks)
					   ;
		}
		mets.in_fee_msat_per_day = ( double(stats.in_fee.to_msat())
					   * 86400
					   )
					 / time
					 ;
		mets.out_fee_msat_per_day = ( double(stats.out_fee.to_msat())
					    * 86400
					    )
					  / time
					  ;
		return mets;
	}
	Ev::Io<Msg::ResponsePeerMetrics>
	get_metrics() {
		return Ev::lift().then([this]() {
			auto end = Ev::now();
			auto f = [this, end](double time) {
				auto start = end - time;
				return get_stats(start, end);
			};
			auto static const times = std::vector<double>{
				86400 * 3,
				86400 * 14,
				86400 * 30,
			};
			return Ev::map(std::move(f), times);
		}).then([](std::vector<std::map< Ln::NodeId
					       , Msg::PeerStatistics
					       >> stats) {
			auto mets = std::vector<std::map< Ln::NodeId
							, Msg::PeerMetrics
							>>(stats.size());
			std::transform( stats.begin(), stats.end()
				      , mets.begin()
				      , &map_stats_to_mets
				      );
			return Ev::lift(Msg::ResponsePeerMetrics{
				nullptr,
				std::move(mets[0]),
				std::move(mets[1]),
				std::move(mets[2])
			});
		});
	}

	Ev::Io<void> run(Msg::RequestPeerMetrics const& req) {
		return get_metrics().then([this, req
					  ](Msg::ResponsePeerMetrics resp) {
			resp.requester = req.requester;
			return bus.raise(std::move(resp));
		});
	}

	Json::Out provide_metrics(Msg::ResponsePeerMetrics const& resp) {
		auto const& data = resp.day3;

		auto out = Json::Out();
		auto obj = out.start_object();

		for (auto& e : data) {
			auto& mets = e.second;
			auto mets_j = Json::Out();
			auto mets_o = mets_j.start_object();

			mets_o.field("age", mets.age);
			mets_o.field( "seconds_per_attempt"
				    , mets.seconds_per_attempt
				    );
			mets_o.field( "success_per_attempt"
				    , mets.success_per_attempt
				    );
			mets_o.field( "success_per_day"
				    , mets.success_per_day
				    );
			mets_o.field( "connect_rate"
				    , mets.connect_rate
				    );
			mets_o.field( "in_fee_msat_per_day"
				    , mets.in_fee_msat_per_day
				    );
			mets_o.field( "out_fee_msat_per_day"
				    , mets.out_fee_msat_per_day
				    );
			mets_o.end_object();

			auto& node = e.first;
			obj.field( std::string(node)
				 , std::move(mets_j)
				 );
		}

		obj.end_object();
		return out;
	}
};

PeerMetrician::PeerMetrician(PeerMetrician&&) =default;
PeerMetrician::~PeerMetrician() =default;

PeerMetrician::PeerMetrician(S::Bus& bus)
		: pimpl(Util::make_unique<Impl>(bus)) { }

}}
