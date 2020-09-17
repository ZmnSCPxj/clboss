#ifndef BOSS_MOD_CHANNELCREATOR_MAIN_HPP
#define BOSS_MOD_CHANNELCREATOR_MAIN_HPP

#include<memory>

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator {
	class Main;
}}}
namespace Boss { namespace Mod { namespace ChannelCreator {
	class Carpenter;
}}}
namespace Boss { namespace Mod { namespace ChannelCreator { class Manager; }}}
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace ChannelCreator {

/** class Boss::Mod::ChannelCreator::Main
 *
 * @brief main module for the channel creator.
 *
 * @desc
 * The channel creator gets an amount of onchain
 * funds to distribute, gets proposals from the
 * `ChannelCandidateInvestigator`, and figures out
 * a plan to distribute the funds, then creates
 * the channels.
 */
class Main {
private:
	std::unique_ptr<Carpenter> carpenter;
	std::unique_ptr<Manager> manager;

public:
	Main() =delete;
	Main(Main const&) =delete;
	Main(Main&&) =delete;

	Main( S::Bus& bus
	    , Boss::Mod::ChannelCandidateInvestigator::Main& investigator
	    );
	~Main();
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCREATOR_MAIN_HPP) */
