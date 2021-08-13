#ifndef BOSS_MOD_PEERJUDGE_AGETRACKER_HPP
#define BOSS_MOD_PEERJUDGE_AGETRACKER_HPP

#include"Ev/now.hpp"
#include"Sqlite3/Db.hpp"
#include<functional>
#include<utility>

namespace Ev { template<typename a> class Io; }
namespace Ln { class NodeId; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace PeerJudge {

/** class Boss::Mod::PeerJudge::AgeTracker
 *
 * @brief Sub-module to keep track of the ages
 * of channels with peers by listening for
 * `Boss::Msg::ChannelCreation` and
 * `Boss::Msg::ChannelDestruction` events.
 *
 * @desc This module, and `PeerJudge` in general,
 * assumes that the `lightningd` can only have
 * one "live" channel with the peer.
 * Given the architecture of `lightningd` as of
 * the time of this writing, having multiple
 * live channels per peer is unlikely to be
 * achievable any time soon.
 */
class AgeTracker {
private:
	S::Bus& bus;
	std::function<double()> get_now;

	Sqlite3::Db db;

	void start();

	Ev::Io<void> wait_for_db();
	Ev::Io<void> update(Ln::NodeId node, double time);

public:
	AgeTracker( S::Bus& bus_
		  , std::function<double()> get_now_ = &Ev::now
		  ) : bus(bus_)
		    , get_now(std::move(get_now_))
		    { start(); }

	Ev::Io<double> get_min_age(Ln::NodeId node);
};

}}}

#endif /* BOSS_MOD_PEERJUDGE_AGETRACKER_HPP */
