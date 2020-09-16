#ifndef BOSS_MOD_CHANNELCREATOR_DOWSER_HPP
#define BOSS_MOD_CHANNELCREATOR_DOWSER_HPP

#include<memory>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
namespace Ln { struct Amount; }
namespace Ln { struct NodeId; }

namespace Boss { namespace Mod { namespace ChannelCreator {

/** class Boss::Mod::ChannelCreator::Dowser
 *
 * @brief given a proposal node and its patron,
 * determines some recommended amount to put
 * into a channel with the proposal.
 */
class Dowser {
private:
	class Impl;
	std::shared_ptr<Impl> pimpl;

public:
	Dowser() =delete;

	Dowser(Dowser&&) =default;
	Dowser(Dowser const&) =default;
	~Dowser() =default;

	Dowser( Boss::Mod::Rpc& rpc
	      /* Our own self.  */
	      , Ln::NodeId const& self
	      /* The node we want.  */
	      , Ln::NodeId const& proposal
	      /* The reason we want the above node.  */
	      , Ln::NodeId const& patron
	      );

	Ev::Io<Ln::Amount> run();
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCREATOR_DOWSER_HPP) */

