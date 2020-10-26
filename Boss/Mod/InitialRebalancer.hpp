#ifndef BOSS_MOD_INITIALREBALANCER_HPP
#define BOSS_MOD_INITIALREBALANCER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::InitialRebalancer
 *
 * @brief If a channel has most funds owned by us at `ListpeersResult`,
 * move funds to other channels if possible.
 */
class InitialRebalancer {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	InitialRebalancer() =delete;

	InitialRebalancer(InitialRebalancer&&);
	~InitialRebalancer();

	explicit
	InitialRebalancer(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_INITIALREBALANCER_HPP) */
