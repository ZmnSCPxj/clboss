#ifndef EV_SEMAPHORE_HPP
#define EV_SEMAPHORE_HPP

#include"Ev/Io.hpp"
#include"Util/make_unique.hpp"
#include<cstddef>
#include<memory>
#include<utility>

namespace Ev { template<typename a> class Io; }

namespace Ev {

namespace Detail {

template<typename a>
struct SemaphoreRunHelper {
	template<typename f>
	static
	Ev::Io<a> run(Ev::Io<a> action, f core_run) {
		/* Store the result in a newly-allocated space.  */
		auto ppresult = std::make_shared<std::unique_ptr<a>>();
		auto void_action = std::move(action).then([ppresult](a rv) {
			/* Move it into the space.  */
			*ppresult = Util::make_unique<a>(std::move(rv));
			return Ev::lift();
		});
		return core_run(std::move(void_action)).then([ppresult]() {
			/* Recover the result.  */
			return Ev::lift(std::move(**ppresult));
		});
	}
};

template<>
struct SemaphoreRunHelper<void> {
	template<typename f>
	static
	Ev::Io<void> run(Ev::Io<void> action, f core_run) {
		return core_run(std::move(action));
	}
};

}

/** class Ev::Semaphore
 *
 * @brief object that allows up to a certain
 * maximum number of simultaneous running
 * greenthreads, blocking extra threads until
 * the others have completed or thrown.
 */
class Semaphore {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

	Ev::Io<void> core_run(Ev::Io<void> action);

public:
	Semaphore() =delete;
	Semaphore(Semaphore const&) =delete;

	Semaphore(Semaphore&& o);
	~Semaphore();
	explicit
	Semaphore(std::size_t max);

	/*~
	 * Execute the given action, but ensure that
	 * only the preconfigured max number are
	 * running simultaneously.
	 *
	 * This will block (and let other greenthreads
	 * run) if the preconfigured max number are
	 * already running concurrently.
	 * You should assume that other greenthreads
	 * will run while this is running.
	 *
	 * This action will throw if the given action
	 * also throws.
	 */
	template<typename a>
	Ev::Io<a> run(Ev::Io<a> action) {
		return Detail::SemaphoreRunHelper<a>::run(std::move(action), [this](Ev::Io<void> action) {
			return core_run(std::move(action));
		});
	}
};

}

#endif /* !defined(EV_SEMAPHORE_HPP) */
