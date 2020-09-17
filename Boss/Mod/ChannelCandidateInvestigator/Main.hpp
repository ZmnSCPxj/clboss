#ifndef BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_MAIN_HPP
#define BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_MAIN_HPP

#include"Ln/NodeId.hpp"
#include<memory>
#include<utility>
#include<vector>

namespace Boss { namespace Mod { class InternetConnectionMonitor; }}
namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator {

class Gumshoe;
class Secretary;
class Manager;

class Main {
private:
	std::unique_ptr<Secretary> secretary;
	std::unique_ptr<Gumshoe> gumshoe;
	std::unique_ptr<Manager> manager;

public:
	Main() =delete;
	Main(Main&&);
	~Main();

	explicit
	Main( S::Bus& bus
	    , Boss::Mod::InternetConnectionMonitor& imon
	    );

	/** Main::get_channel_candidates
	 *
	 * @brief gets the channel candidates, in order from
	 * best score to worst score.
	 * `.first` is the actual node to channel with,
	 * `.second` is the reason why we think the actual node
	 * is a good idea.
	 */
	Ev::Io<std::vector<std::pair<Ln::NodeId, Ln::NodeId>>>
	get_channel_candidates();
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_MAIN_HPP) */
