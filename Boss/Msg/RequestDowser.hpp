#ifndef BOSS_MSG_REQUESTDOWSER_HPP
#define BOSS_MSG_REQUESTDOWSER_HPP

#include"Ln/NodeId.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestDowser
 *
 * @brief Requests for a dowser estimation of the flow between
 * two nodes.
 *
 * @desc The dowser just guesses how much capacity is available
 * between the two specified nodes.
 */
struct RequestDowser {
	void* requester;
	Ln::NodeId a;
	Ln::NodeId b;
};

}}

#endif /* !defined(BOSS_MSG_REQUESTDOWSER_HPP) */
