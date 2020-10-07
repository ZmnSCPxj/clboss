#ifndef BOSS_MSG_PROBEACTIVELY_HPP
#define BOSS_MSG_PROBEACTIVELY_HPP

#include"Ln/NodeId.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ProbeActively
 *
 * @brief requests the active prober to initiate an attempt
 * at actively probing the given peer.
 */
struct ProbeActively {
	Ln::NodeId peer;
};

}}

#endif /* !defined(BOSS_MSG_PROBEACTIVELY_HPP) */
