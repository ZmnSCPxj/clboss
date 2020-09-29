#ifndef BOSS_MOD_NEEDSONCHAINFUNDSSWAPPER_HPP
#define BOSS_MOD_NEEDSONCHAINFUNDSSWAPPER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::NeedsOnchainFundsSwapper
 *
 * @brief Handles `Boss::Msg::NeedsOnchainFunds`
 * messages, by swapping offchain funds for
 * onchain funds.
 */
class NeedsOnchainFundsSwapper {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	NeedsOnchainFundsSwapper() =delete;

	NeedsOnchainFundsSwapper(NeedsOnchainFundsSwapper&&);
	NeedsOnchainFundsSwapper& operator=(NeedsOnchainFundsSwapper&&);
	~NeedsOnchainFundsSwapper();

	explicit
	NeedsOnchainFundsSwapper(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_NEEDSONCHAINFUNDSSWAPPER_HPP) */
