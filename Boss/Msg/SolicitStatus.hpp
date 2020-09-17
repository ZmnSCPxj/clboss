#ifndef BOSS_MSG_SOLICITSTATUS
#define BOSS_MSG_SOLICITSTATUS

namespace Boss { namespace Msg {

/** struct Boss::Msg::SolicitStatus
 *
 * @brief emitted whenever the user issues a
 * `clboss-status` command.
 * Modules should respond synchronously with
 * `Boss::Msg::ProvideStatus` messages.
 */
struct SolicitStatus {};

}}

#endif /* !defined(BOSS_MSG_SOLICITSTATUS) */
