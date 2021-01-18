#ifndef BOSS_MOD_FEEMODDERBYFORWARDER_HPP
#define BOSS_MOD_FEEMODDERBYFORWARDER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::FeeModderByForwarder
 *
 * @brief Modifies channel fees simply because we are a
 * forwarding node.
 *
 * @desc The logic here is that a forwarder is really in
 * the business of aggregating multiple small payments.
 * This is because the actual payments made by payers is
 * determined by the actual transaction (payment for a
 * product or service).
 *
 * However, as a forwarding node, we (presumably!) have a
 * good amount of capacity in our channels, and can service
 * many payments through them.
 * So our strategy is to charge the many payments going
 * through us by boosting our base fee and lowering our
 * proportional fee.
 * This allows us to afford to perform rebalances.
 *
 * In effect, the many payments going through our node are
 * aggregated into a smaller number of rebalances which
 * effectively take more expensive paths.
 */
class FeeModderByForwarder {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	FeeModderByForwarder() =delete;

	FeeModderByForwarder(FeeModderByForwarder&&);
	~FeeModderByForwarder();

	explicit
	FeeModderByForwarder(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_FEEMODDERBYFORWARDER_HPP) */
