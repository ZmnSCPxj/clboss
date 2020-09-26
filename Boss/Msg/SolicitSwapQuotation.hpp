#ifndef BOSS_MSG_SOLICITSWAPQUOTATION_HPP
#define BOSS_MSG_SOLICITSWAPQUOTATION_HPP

#include"Ln/Amount.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::SolicitSwapQuotation
 *
 * @brief Broadcasted whenever the swap manager wants to get
 * quotes on swapping offchain funds for onchain funds.
 * Modules listening to this message must reply synchronously
 * with `Boss::Msg::ProvideSwapQuotation` messages.
 */
struct SolicitSwapQuotation {
	/* How much offchain amount we want to send out.  */
	Ln::Amount offchain_amount;
	/* The module asking for quotation.
	 * This is replicated as well in the
	 * `Boss::Msg::ProvideSwapQuotation`
	 * message to indicate which solicit
	 * is being responded to.
	 */
	void* solicitor;
};

}}

#endif /* !defined(BOSS_MSG_SOLICITSWAPQUOTATION_HPP) */
