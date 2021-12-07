#ifndef BOSS_MOD_CHANNELCANDIDATEMATCHMAKER_HPP
#define BOSS_MOD_CHANNELCANDIDATEMATCHMAKER_HPP

#include"Boss/Msg/PatronizeChannelCandidate.hpp"
#include"Ln/Amount.hpp"
#include<queue>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ChannelCandidateMatchmaker
 *
 * @brief matches up patronless channel candidates to
 * potential patrons.
 *
 * @desc more specifically, it waits for the channel
 * candidate investigator to convert messages of
 * `Boss::Msg::ProposePatronlessChannelCandidate` to
 * `Boss::Msg::PatronizeChannelCandidate`, and uses
 * the provided guide to help derive the patron it
 * proposes.
 *
 * If it is able to find a patron, this module emits
 * `Boss::Msg::ProposeChannelCandidates`.
 */
class ChannelCandidateMatchmaker {
private:
	class Run;

	S::Bus& bus;
	Boss::Mod::Rpc* rpc;

	Ln::Amount min_channel;

	void start();

public:
	ChannelCandidateMatchmaker() =delete;
	ChannelCandidateMatchmaker(ChannelCandidateMatchmaker&&) =delete;
	ChannelCandidateMatchmaker(ChannelCandidateMatchmaker const&) =delete;

	explicit
	ChannelCandidateMatchmaker(S::Bus& bus_
				  ) : bus(bus_)
				    , rpc(nullptr)
				    { start(); }
};

}}

#endif /* !defined(BOSS_MOD_CHANNELCANDIDATEMATCHMAKER_HPP) */
