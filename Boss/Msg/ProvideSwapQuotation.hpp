#ifndef BOSS_MSG_PROVIDESWAPQUOTATION_HPP
#define BOSS_MSG_PROVIDESWAPQUOTATION_HPP

#include"Ln/Amount.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ProvideSwapQuotation
 *
 * @brief broadcasted in response to handling a
 * `Boss::Msg::SolicitSwapQuotation` message.
 * If your module is selected, then a later
 * `Boss::Msg::AcceptSwapQuotation` message will
 * be broadcast identifying your module via the
 * given provider pointer.
 */
struct ProvideSwapQuotation {
	/* Estimate of how much will be deducted
	 * from the offchain amount to provide
	 * the onchain amount.
	 */
	Ln::Amount fee;

	/* Identify the solicitor we are responding
	 * to.
	 */
	void* solicitor;
	/* Identify the module that is responding.  */
	void* provider;
};

}}

#endif /* !defined(BOSS_MSG_PROVIDESWAPQUOTATION_HPP) */
