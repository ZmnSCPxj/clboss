#ifndef BOSS_MSG_ONCHAINFUNDS_HPP
#define BOSS_MSG_ONCHAINFUNDS_HPP

#include"Ln/Amount.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::OnchainFunds
 *
 * @brief message sent whenever we notice
 * we have some confirmed onchain funds.
 *
 * @desc the amount indicated already has the
 * cost of spending the inputs and creating one
 * channel factored in,
 * and will thus be lower if your funds are in
 * many small inputs than if it were in a single
 * big input.
 *
 * Only deeply-confirmed funds are considered.
 * This includes change outputs.
 */
struct OnchainFunds {
	Ln::Amount onchain;
};

}}

#endif /* !defined(BOSS_MSG_ONCHAINFUNDS_HPP) */
