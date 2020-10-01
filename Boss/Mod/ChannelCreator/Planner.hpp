#ifndef BOSS_MOD_CHANNELCREATOR_PLANNER_HPP
#define BOSS_MOD_CHANNELCREATOR_PLANNER_HPP

#include<cstddef>
#include<functional>
#include<map>
#include<memory>
#include<utility>
#include<vector>

namespace Ev { template<typename a> class Io; }
namespace Ln { class Amount; }
namespace Ln { class NodeId; }

namespace Boss { namespace Mod { namespace ChannelCreator {

/** class Boss::Mod::ChannelCreator::Planner
 *
 * @brief creates a plan for how to distribute an
 * amount of bitcoins to channels to proposal
 * nodes.
 *
 * @desc the plan can contain nodes with 0 amount,
 * meaning that node is too badly-connected to
 * make channels with, and we should instead drop
 * the node from channel proposals.
 */
class Planner {
private:
	class Impl;
	std::shared_ptr<Impl> pimpl;

public:
	Planner() =delete;

	Planner(Planner const&) =default;
	Planner(Planner&&) =default;
	~Planner() =default;

	typedef
	std::function<Ev::Io<Ln::Amount>(Ln::NodeId, Ln::NodeId)>
	DowserFunc;

	       /* Function to recommend how much funds to give to a
		* proposal-patron pair.  */
	Planner( DowserFunc dowser_func
	       /* Total amount to distribute.  */
	       , Ln::Amount amount
	       /* Nodes to plan to channel with.
		* Order in priority, i.e. [0] is highest priority
		* for making channel with.
		*/
	       , std::vector<std::pair<Ln::NodeId, Ln::NodeId>> proposals
	       /* Number of existing channels.  */
	       , std::size_t existing_channels
	       /* Limits on amounts.  */
	       , Ln::Amount min_amount
	       , Ln::Amount max_amount
	       /* If we cannot assign funds to all proposals, how
		* much minimum to leave over.  */
	       , Ln::Amount min_remaining
	       );

	Ev::Io<std::map<Ln::NodeId, Ln::Amount>>
	run() &&;
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCREATOR_PLANNER_HPP) */

