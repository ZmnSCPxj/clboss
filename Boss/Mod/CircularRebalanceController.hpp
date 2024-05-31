#ifndef BOSS_MOD_CIRCULARREBALANCECONTROLLER_HPP
#define BOSS_MOD_CIRCULARREBALANCECONTROLLER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::CircularRebalanceController
 *
 * @brief Provides RPC commands `clboss-circular-rebalance` and
 * `clboss-temporarily-circular-rebalance` to tell CLBOSS whether or not to
 * allow circular rebalancing.
 * Also exposes the state to other modules via the
 * `Msg::RequestGetCircularRebalanceFlag` message.
 */
class CircularRebalanceController {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	CircularRebalanceController() =delete;
	CircularRebalanceController(CircularRebalanceController const&) =delete;

	CircularRebalanceController(CircularRebalanceController&&);
	~CircularRebalanceController();

	explicit
	CircularRebalanceController(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_CIRCULARREBALANCECONTROLLER_HPP) */
