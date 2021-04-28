#ifndef BOSS_MSG_SOLICITDELETABLEPAYMENTLABELFILTER_HPP
#define BOSS_MSG_SOLICITDELETABLEPAYMENTLABELFILTER_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::SolicitDeletablePaymentLabelFilter
 *
 * @brief Broadcast over the bus to ask for filter functions
 * to determine if we should delete a completed/failed
 * payment.
 * Clients should respond with
 * `Boss::Msg::ProvideDeletablePaymentLabelFilter`.
 */
struct SolicitDeletablePaymentLabelFilter {};

}}

#endif /* !defined(BOSS_MSG_SOLICITDELETABLEPAYMENTLABELFILTER_HPP) */
