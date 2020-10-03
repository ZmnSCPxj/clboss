#ifndef BOSS_MSG_RELEASEHTLCACCEPTED_HPP
#define BOSS_MSG_RELEASEHTLCACCEPTED_HPP

#include"Ln/HtlcAccepted.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ReleaseHtlcAccepted
 *
 * @brief emitted by a module when it has decided on
 * how to handle an `htlc_accepted` hook that it
 * previously deferred.
 *
 * @desc `Boss::Mod::HtlcAcceptor` implements a timeout
 * on deferred HTLCs, and if the timeout is reached,
 * will let the HTLC be handled normally by `lightningd`.
 * If you send after the timeout, the acceptor will
 * safely ignore it.
 *
 * HTLCs are identified by the `id` in the request that
 * is passed to deferrer functions that were provided in
 * `Boss::Msg::ProvideHtlcAcceptedDeferrer` messages.
 */
struct ReleaseHtlcAccepted {
	Ln::HtlcAccepted::Response response;
};

}}

#endif /* !defined(BOSS_MSG_RELEASEHTLCACCEPTED_HPP) */
