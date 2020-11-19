#ifndef BOSS_MOD_ONCHAINFUNDSIGNORER_HPP
#define BOSS_MOD_ONCHAINFUNDSIGNORER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::OnchainFundsIgnorer
 *
 * @brief Provides RPC commands `clboss-ignore-onchain` and
 * `clboss-notice-onchain` to tell CLBOSS to ignore onchain
 * funds.
 * Also exposes the `ignore` flag to other modules via the
 * `Msg::RequestGetIgnoreOnchainFlag` message.
 */
class OnchainFundsIgnorer {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	OnchainFundsIgnorer() =delete;
	OnchainFundsIgnorer(OnchainFundsIgnorer const&) =delete;

	OnchainFundsIgnorer(OnchainFundsIgnorer&&);
	~OnchainFundsIgnorer();

	explicit
	OnchainFundsIgnorer(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_ONCHAINFUNDSIGNORER_HPP) */
