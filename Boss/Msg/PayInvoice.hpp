#ifndef BOSS_MSG_PAYINVOICE_HPP
#define BOSS_MSG_PAYINVOICE_HPP

#include<string>

namespace Boss { namespace Msg {

/** struct Boss::Msg::PayInvoice
 *
 * @brief Emitted when we want to pay an
 * invoice for some reason.
 */
struct PayInvoice {
	std::string invoice;
};

}}

#endif /* !defined(BOSS_MSG_PAYINVOICE_HPP) */
