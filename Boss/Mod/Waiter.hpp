#ifndef BOSS_MOD_WAITER_HPP
#define BOSS_MOD_WAITER_HPP

#include<memory>

namespace Ev { template<typename a> class Io;}
namespace S { class Bus; }

namespace Boss { namespace Mod {

class Waiter {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	explicit
	Waiter(S::Bus& bus);
	~Waiter();

	/** Boss::Mod::Waiter::wait
	 *
	 * @brief Waits for the specified number of
	 * seconds, then the action returns.
	 * Throws a Boss::Shutdown exception if a
	 * shutdown is broadcasted on the bus.
	 */
	Ev::Io<void> wait(double seconds);
};

}}

#endif /* !defined(BOSS_MOD_WAITER_HPP) */
