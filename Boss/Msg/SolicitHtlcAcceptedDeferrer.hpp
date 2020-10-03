#ifndef BOSS_MSG_SOLICITHTLCACCEPTEDDEFERRER_HPP
#define BOSS_MSG_SOLICITHTLCACCEPTEDDEFERRER_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::SolicitHtlcAcceptedDeferrer
 *
 * @brief broadcast to inform all modules that we want to
 * get `Boss::Msg::ProvideHtlcAcceptedDeferrer` messages.
 * Modules that want to register their deferrer functions
 * should synchronously reply to this message.
 */
struct SolicitHtlcAcceptedDeferrer {};

}}

#endif /* !defined(BOSS_MSG_SOLICITHTLCACCEPTEDDEFERRER_HPP) */
