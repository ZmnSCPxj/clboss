#ifndef BOSS_MSG_INTERNETONLINE_HPP
#define BOSS_MSG_INTERNETONLINE_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::InternetOnline
 *
 * @brief signals to everyone whether we are
 * online or not.
 *
 * @desc Broadcast whenever online-ness changes.
 *
 * At startup, we are considered offline, so the
 * first `Boss::Msg::InternetOnline` would have
 * `online` as `true`.
 * Conversely, if you are tracking this in your
 * module, you can initialize your module to
 * assume offline-ness, and once we do our
 * initial online check the
 * `Boss::Msg::InternetOnline` will update your
 * module if we are online after all.
 */
struct InternetOnline {
	bool online;
};

}}

#endif /* !defined(BOSS_MSG_INTERNETONLINE_HPP) */
