#ifndef BOSS_MSG_SOLICITUNMANAGEMENT_HPP
#define BOSS_MSG_SOLICITUNMANAGEMENT_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::SolicitUnmanagement
 *
 * @brief Asks other modules to register their unmanagement tags to the
 * `Boss::Mod::UnmanagedManager`.
 * Modules should immediately respond with a `ProvideUnmanagement`
 * message.
 */
struct SolicitUnmanagement { };

}}

#endif /* !defined(BOSS_MSG_SOLICITUNMANAGEMENT_HPP) */
