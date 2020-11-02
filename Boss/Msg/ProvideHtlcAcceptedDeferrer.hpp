#ifndef BOSS_MSG_PROVIDEHTLCACCEPTED_DEFERRER_HPP
#define BOSS_MSG_PROVIDEHTLCACCEPTED_DEFERRER_HPP

#include<functional>

namespace Ev { template<typename a> class Io; }
namespace Ln { namespace HtlcAccepted { struct Request; }}

namespace Boss { namespace Msg {

/** struct Boss::Msg::ProvideHtlcAcceptedDeferrer
 *
 * @brief Broadcast by modules synchronously in response to
 * `Boss::Msg::SolicitHtlcAcceptedDeferrer`.
 *
 * @desc The function `deferrer` will be remembered by the
 * `Boss::Mod::HtlcAcceptor`, and whenever an `htlc_accepted`
 * hook is triggered, will be called with the details of the
 * payload.
 * The function returns `false` if it does not want to handle
 * the HTLC, `true` if it wants to handle the HTLC specially.
 *
 * If the function returns `true`, it is a promise by the
 * providing module that it *will* broadcast a later
 * `Boss::Msg::ReleaseHtlcAccepted`.
 */
struct ProvideHtlcAcceptedDeferrer {
	std::function<Ev::Io<bool>(Ln::HtlcAccepted::Request const&)>
	deferrer;
};

}}

#endif /* !defined(BOSS_MSG_PROVIDEHTLCACCEPTED_DEFERRER_HPP) */
