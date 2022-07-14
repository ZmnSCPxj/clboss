#ifndef BOSS_MOD_REBALANCEBUDGETER_HPP
#define BOSS_MOD_REBALANCEBUDGETER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::RebalanceBudgeter
 *
 * @brief Figures out how much to allocate
 * towards rebalances.
 */
class RebalanceBudgeter {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	RebalanceBudgeter() =delete;

	RebalanceBudgeter(RebalanceBudgeter&&);
	~RebalanceBudgeter();

	/* Formal constructor.  */
	explicit
	RebalanceBudgeter(S::Bus&);
};

}}

#endif /* !defined(BOSS_MOD_REBALANCEBUDGETER_HPP) */
