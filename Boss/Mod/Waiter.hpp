#ifndef BOSS_MOD_WAITER_HPP
#define BOSS_MOD_WAITER_HPP

#include"Ev/Io.hpp"
#include"Util/make_unique.hpp"
#include<memory>
#include<stdexcept>

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

	/** Boss::Mod::Waiter::timed
	 *
	 * @brief performs the action, but if it does
	 * not complete before the given timeout,
	 * throws a `TimedOut` exception within the
	 * `Ev::Io` system.
	 *
	 * @desc `Ev::Io` does not really allow for
	 * cancelling of ongoing tasks, so the given
	 * action will still run to completion even if
	 * the timeout is reached, but the result will
	 * be destructed as soon as the action
	 * completes.
	 */
	template<typename a>
	Ev::Io<a> timed( double timeout
		       , Ev::Io<a> action
		       );
	struct TimedOut { };

private:
	Ev::Io<void> timed_core( double timeout
			       , Ev::Io<void> action
			       );
};

template<typename a>
inline
Ev::Io<a> Waiter::timed( double timeout
		       , Ev::Io<a> action
		       ) {
	auto paction = std::make_shared<Ev::Io<a>>(
		std::move(action)
	);
	auto presult = std::make_shared<std::unique_ptr<a>>();
	return timed_core(timeout, Ev::lift().then([paction, presult]() {
		return std::move(*paction).then([presult](a value) {
			*presult = Util::make_unique<a>(
				std::move(value)
			);
			return Ev::lift();
		});
	})).then([presult]() {
		return Ev::lift(std::move(**presult));
	});
}
template<>
inline
Ev::Io<void> Waiter::timed<void>( double timeout
				, Ev::Io<void> action
				) {
	return timed_core(timeout, action);
}

}}

#endif /* !defined(BOSS_MOD_WAITER_HPP) */
