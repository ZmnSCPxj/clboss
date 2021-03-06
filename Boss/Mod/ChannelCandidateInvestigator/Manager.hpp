#ifndef BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_MANAGER_HPP
#define BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_MANAGER_HPP

#include"Sqlite3/Db.hpp"
#include"Ln/NodeId.hpp"
#include<cstddef>
#include<set>
#include<utility>
#include<vector>

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator { class Gumshoe; }}}
namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator { class Secretary; }}}
namespace Boss { namespace Mod { class InternetConnectionMonitor; }}
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator {

/** class Boss::Mod::ChannelCandidateInvestigator::Manager
 *
 * @brief performs the heuristics needed to operate
 * the investigator.
 */
class Manager {
private:
	S::Bus& bus;
	Secretary& secretary;
	Gumshoe& gumshoe;
	InternetConnectionMonitor& imon;

	Sqlite3::Db db;

	std::set<Ln::NodeId> unmanaged;

	void start();
	Ev::Io<void> solicit_candidates(std::size_t good_candidates);

public:
	Manager() =delete;
	Manager(Manager const&) =delete;
	Manager(Manager&&) =delete;

	explicit
	Manager( S::Bus& bus_
	       , Secretary& secretary_
	       , Gumshoe& gumshoe_
	       , InternetConnectionMonitor& imon_
	       ) : bus(bus_)
		 , secretary(secretary_)
		 , gumshoe(gumshoe_)
		 , imon(imon_)
		 {
		start();
	}

	Ev::Io<std::vector<std::pair<Ln::NodeId, Ln::NodeId>>>
	get_channel_candidates();
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_MANAGER_HPP) */
