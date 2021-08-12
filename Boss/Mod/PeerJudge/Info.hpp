#ifndef BOSS_MOD_PEERJUDGE_INFO_HPP
#define BOSS_MOD_PEERJUDGE_INFO_HPP

#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"

namespace Boss { namespace Mod { namespace PeerJudge {

/** struct Boss::Mod::PeerJudge::Info
 *
 * @brief Contains information gathered about one peer.
 */
struct Info {
	Ln::NodeId id;

	/* The size of all CHANNELD_NORMAL channels to the peer.  */
	Ln::Amount total_normal;
	/* The total earnings over the specified time frame.  */
	Ln::Amount earned;
};

}}}

#endif /* !defined(BOSS_MOD_PEERJUDGE_INFO_HPP) */
