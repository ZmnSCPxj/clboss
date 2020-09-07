#ifndef BOSS_MSG_MANIFESTATION_HPP
#define BOSS_MSG_MANIFESTATION_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::Manifestation
 *
 * @brief emitted during `getmanifest`.
 * Modules triggering on this message should emit
 * Boss::Msg::Manifest* messages.
 */
struct Manifestation {};

}}

#endif /* BOSS_MSG_MANIFESTATION_HPP */
