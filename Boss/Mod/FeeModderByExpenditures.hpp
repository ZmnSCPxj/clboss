#ifndef BOSS_MOD_FEEMODDERBYEXPENDITURES_HPP
#define BOSS_MOD_FEEMODDERBYEXPENDITURES_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::FeeModderByExpenditures
 *
 * @brief Modify fees according to expenditures.
 * If we have lots of expenditures, increase the fee
 * we ask for.
 */
class FeeModderByExpenditures {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	FeeModderByExpenditures() =delete;

	FeeModderByExpenditures(FeeModderByExpenditures&&);
	~FeeModderByExpenditures();

	explicit
	FeeModderByExpenditures(S::Bus&);
};

}}

#endif /* !defined(BOSS_MOD_FEEMODDERBYEXPENDITURES_HPP) */
