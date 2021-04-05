#ifndef BOSS_MOD_CHANNELCREATOR_REARRANGERBYSIZE_HPP
#define BOSS_MOD_CHANNELCREATOR_REARRANGERBYSIZE_HPP

#include<functional>
#include<memory>
#include<utility>
#include<vector>

namespace Ev { template<typename a> class Io; }
namespace Ln { class Amount; }
namespace Ln { class NodeId; }

namespace Boss { namespace Mod { namespace ChannelCreator {

/** class Boss::Mod::ChannelCreator::RearrangerBySize
 *
 * @brief Mildly rearranges the input proposals according to the
 * dowsed capacity between the proposal and its patron.
 */
class RearrangerBySize {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	RearrangerBySize() =delete;

	RearrangerBySize(RearrangerBySize&&);
	~RearrangerBySize();

	typedef
	std::function<Ev::Io<Ln::Amount>(Ln::NodeId, Ln::NodeId)>
	DowserFunc;

	explicit
	RearrangerBySize(DowserFunc dowser_func);

	Ev::Io<std::vector<std::pair<Ln::NodeId, Ln::NodeId>>>
	rearrange_by_size(std::vector< std::pair<Ln::NodeId, Ln::NodeId>
				     > proposals);
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCREATOR_REARRANGERBYSIZE_HPP) */
