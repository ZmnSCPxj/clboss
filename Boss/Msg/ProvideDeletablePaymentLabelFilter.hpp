#ifndef BOSS_MSG_PROVIDEDELETABLEPAYMENTLABELFILTER_HPP
#define BOSS_MSG_PROVIDEDELETABLEPAYMENTLABELFILTER_HPP

#include<functional>
#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::ProvideDeletablePaymentLabelFilter
 *
 * @brief Sent in direct response to the
 * `Boss::Msg::SolicitDeletablePaymentLabelFilter`
 * message.
 * This provides a function that, if it returns true,
 * indicates the payment with the given label should
 * be deleted.
 */
struct ProvideDeletablePaymentLabelFilter {
	std::function<bool(std::string const&)> filter;
};

}}

#endif /* !defined(BOSS_MSG_PROVIDEDELETABLEPAYMENTLABELFILTER_HPP) */

