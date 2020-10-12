#ifndef BOSS_MSG_RESPONSEDOWSER_HPP
#define BOSS_MSG_RESPONSEDOWSER_HPP

#include"Ln/Amount.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ResponseDowser
 *
 * @brief Emitted in response to `Boss::Msg::RequestDowser`.
 *
 * @desc this contains the estimated practical capacity between the two
 * nodes.
 */
struct ResponseDowser {
	void* requester;
	Ln::Amount amount;
};

}}

#endif /* !defined(BOSS_MSG_RESPONSEDOWSER_HPP) */
