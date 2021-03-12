#ifndef BOSS_MOD_FEEMODDERBYPRICETHEORY_HPP
#define BOSS_MOD_FEEMODDERBYPRICETHEORY_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::FeeModderByPriceTheory
 *
 * @brief controls fee according to a theory that there exists
 * some global optimum price that maximizes profit.
 */
class FeeModderByPriceTheory {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	FeeModderByPriceTheory() =delete;

	FeeModderByPriceTheory(FeeModderByPriceTheory&&);
	~FeeModderByPriceTheory();

	explicit
	FeeModderByPriceTheory(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_FEEMODDERBYPRICETHEORY_HPP) */
