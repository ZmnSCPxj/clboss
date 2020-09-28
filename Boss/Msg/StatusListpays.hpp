#ifndef BOSS_MSG_STATUSLISTPAYS_HPP
#define BOSS_MSG_STATUSLISTPAYS_HPP

namespace Boss { namespace Msg {

/** enum Boss::Msg::StatusListpays
 *
 * @brief the possible status of some payment.
 */
enum StatusListpays
{ StatusListpays_nonexistent
, StatusListpays_success
, StatusListpays_pending
, StatusListpays_failed
};

}}

#endif /* !defined(BOSS_MSG_STATUSLISTPAYS_HPP) */
