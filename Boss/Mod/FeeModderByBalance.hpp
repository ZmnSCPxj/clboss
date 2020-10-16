#ifndef BOSS_MOD_FEEMODDERBYBALANCE_HPP
#define BOSS_MOD_FEEMODDERBYBALANCE_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::FeeModderByBalance
 *
 * @brief modifies fees according to imbalance of
 * channel.
 *
 * @desc
 * The more funds we have in the channel, the lower
 * the fee we put, and the less funds we have in
 * the channel, the higher the fee.
 *
 * The intent is that this helps signal as well to
 * our neighbors that rebalancing via our imbalanced
 * channels in order to improve their own active
 * balancing would be appreciated.
 */
class FeeModderByBalance {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	FeeModderByBalance() =delete;

	~FeeModderByBalance();
	FeeModderByBalance(FeeModderByBalance&&);

	explicit
	FeeModderByBalance(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_FEEMODDERBYBALANCE_HPP) */
