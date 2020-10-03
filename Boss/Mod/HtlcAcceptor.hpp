#ifndef BOSS_MOD_HTLCACCEPTOR_HPP
#define BOSS_MOD_HTLCACCEPTOR_HPP

#include<memory>

namespace Boss { namespace Mod { class Waiter; }}
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::HtlcAcceptor
 *
 * @brief manifests an interest in the `htlc_accepted`
 * hook, and allows other modules to defer the normal
 * handling of `htlc_accepted`.
 *
 * @desc See `Boss::Msg::SolicitHtlcAcceptedDeferrer`,
 * `Boss::Msg::ProvideHtlAcceptedDeferrer`, and
 * `Boss::Msg::ReleaseHtlcAccepted`.
 */
class HtlcAcceptor {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	HtlcAcceptor() =delete;

	HtlcAcceptor(HtlcAcceptor&&);
	~HtlcAcceptor();

	explicit
	HtlcAcceptor( S::Bus& bus
		    , Boss::Mod::Waiter& waiter
		    );
};

}}

#endif /* !defined(BOSS_MOD_HTLCACCEPTOR_HPP) */
