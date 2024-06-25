#ifndef BOSS_MOD_AUTOSWAPCONTROLLER_HPP
#define BOSS_MOD_AUTOSWAPCONTROLLER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::AutoSwapController
 *
 * @brief Provides RPC commands `clboss-auto-swap` and
 * `clboss-temporarily-auto-swap` to tell CLBOSS whether or not to allow
 * offchain-to-onchain swaps.
 * Also exposes the state to other modules via the
 * `Msg::RequestGetAutoSwapFlag` message.
 */
class AutoSwapController {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	AutoSwapController() =delete;
	AutoSwapController(AutoSwapController const&) =delete;

	AutoSwapController(AutoSwapController&&);
	~AutoSwapController();

	explicit
	AutoSwapController(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_AUTOSWAPCONTROLLER_HPP) */
