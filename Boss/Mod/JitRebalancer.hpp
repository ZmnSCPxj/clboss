#ifndef BOSS_MOD_JITREBALANCER_HPP
#define BOSS_MOD_JITREBALANCER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::JitRebalancer
 *
 * @brief Rebalances channels just in time for a forward.
 *
 * @desc Registers as an `htlc_accepted` deferrer, and defers
 * forwards if they are to nodes with insufficient outgoing
 * capacity.
 * It then arranges to move funds to that channel if possible.
 */
class JitRebalancer {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	JitRebalancer() =delete;

	JitRebalancer(JitRebalancer&&);
	~JitRebalancer();

	explicit
	JitRebalancer(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_JITREBALANCER_HPP) */
