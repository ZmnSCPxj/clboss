#ifndef BOSS_MOD_FEEMODDERBYSIZE_HPP
#define BOSS_MOD_FEEMODDERBYSIZE_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::FeeModderBySize
 *
 * @brief Adjust fees depending on the size
 * of our node relative to competitors.
 */
class FeeModderBySize {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	FeeModderBySize() =delete;
	FeeModderBySize(FeeModderBySize&&);
	~FeeModderBySize();

	explicit
	FeeModderBySize(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_FEEMODDERBYSIZE_HPP) */
