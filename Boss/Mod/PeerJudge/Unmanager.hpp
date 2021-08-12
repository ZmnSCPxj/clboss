#ifndef BOSS_MOD_PEERJUDGE_UNMANAGER_HPP
#define BOSS_MOD_PEERJUDGE_UNMANAGER_HPP

#include"Ln/NodeId.hpp"
#include<set>

namespace S { class Bus; }

namespace Boss { namespace Mod { namespace PeerJudge {

/** class Boss::Mod::PeerJudge::Unmanager
 *
 * @brief Holds information on unmanaged `close` nodes.
 */
class Unmanager {
private:
	std::set<Ln::NodeId> unmanaged;

public:
	Unmanager() =delete;
	Unmanager(Unmanager const&) =delete;

	Unmanager(Unmanager&&) =default;

	explicit
	Unmanager(S::Bus& bus);

	std::set<Ln::NodeId> const& get_unmanaged() const {
		return unmanaged;
	}
};

}}}

#endif /* !defined(BOSS_MOD_PEERJUDGE_UNMANAGER_HPP) */
