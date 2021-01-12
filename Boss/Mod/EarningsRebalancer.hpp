#ifndef BOSS_MOD_EARNINGSREBALANCER_HPP
#define BOSS_MOD_EARNINGSREBALANCER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::EarningsRebalancer
 *
 * @brief moves funds to nodes that have low capacity on our side.
 * Determine sources of funds based on earned incoming fees.
 */
class EarningsRebalancer {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	EarningsRebalancer() =delete;

	EarningsRebalancer(EarningsRebalancer&&);
	EarningsRebalancer& operator=(EarningsRebalancer&&) =default;
	~EarningsRebalancer();

	explicit
	EarningsRebalancer(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_EARNINGSREBALANCER_HPP) */
