#ifndef BOSS_MSG_PROVIDEUNMANAGEMENT_HPP
#define BOSS_MSG_PROVIDEUNMANAGEMENT_HPP

#include<functional>
#include<string>

namespace Ev { template<typename a> class Io; }
namespace Ln { class NodeId; }

namespace Boss { namespace Msg {

/** struct Boss::Msg::ProvideUnmanagement
 *
 * @brief Tell the `Boss::Mod::UnmanagedManager` about any unmanagement tags
 * this module wants to use, in response to a broadcasted `SolicitUnmanagement`.
 */
struct ProvideUnmanagement {
	std::string tag;
	/* When initially provided this function is called with `true` for all
	 * nodes with this tag.
	 * Then, when a tag is removed or added, this is called with `true` if
	 * added or `false` if removed.  */
	std::function<Ev::Io<void>(Ln::NodeId const& node, bool unmanaged)> inform;
};

}}

#endif /* !defined(BOSS_MSG_PROVIDEUNMANAGEMENT_HPP) */
