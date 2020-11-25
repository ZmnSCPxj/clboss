#ifndef BOSS_MSG_SOLICITPEERCOMPLAINTS_HPP
#define BOSS_MSG_SOLICITPEERCOMPLAINTS_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::SolicitPeerComplaints
 *
 * @brief emitted periodically to solicit complaints about bad
 * peers from our modules.
 * Modules should emit `Boss::Msg::RaisePeerComplaint` messages
 * in direct reaction to this message.
 */
struct SolicitPeerComplaints {};

}}

#endif /* !defined(BOSS_MSG_SOLICITPEERCOMPLAINTS_HPP) */
