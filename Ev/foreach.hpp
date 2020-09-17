#ifndef EV_FOREACH_HPP
#define EV_FOREACH_HPP

#include"Ev/map.hpp"

namespace Ev {

/** Ev::foreach
 *
 * @brief executes the given function on each
 * item of the input vector, returning an
 * action that yields nothing.
 *
 * @desc each item in the input is moved and
 * passed to the given function.
 * The given function is executed in new
 * greenthreads, so they will overlap in
 * execution time (though note that this is a
 * greenthread library, so compute resource is
 * always a single CPU; this is only useful if
 * I/O is what is delaying you).
 *
 * If the function raises an exception on any
 * item, then all functions will still complete
 * execution (possibly more than one of them
 * can raise an exception), and one of the
 * exceptions raised will be raised after all
 * the functions have finished.
 *
 * The action will not return until all the
 * parallel executions have completed.
 *
 * See also: `Ev::map`.
 */
template<typename f, typename a>
Io<void> foreach(f func, std::vector<a> as);

namespace Detail {

template<typename f, typename a>
class ForeachRun {
private:
	f func;

public:
	ForeachRun() =delete;
	ForeachRun(ForeachRun const&) =default;
	ForeachRun(ForeachRun&&) =default;

	ForeachRun(f func_) : func(std::move(func_)) { }

	Ev::Io<char> operator()(a value) const {
		return func(std::move(value)).then([]() {
			return Ev::lift((char)0);
		});
	}
};

}

template<typename f, typename a>
Io<void> foreach(f func, std::vector<a> as) {
	/* A bit wasteful since it allocates an entire
	 * vector of chars, but the alternative would be
	 * to copy and rewrite a lot of the plumbing
	 * that map already has.
	 * FIXME: copy and rewrite the plumbing of map
	 * so we can get rid of this extra unused vector.
	 */
	return Ev::map( Detail::ForeachRun<f, a>(std::move(func))
		      , std::move(as)
		      ).then([](std::vector<char>) {
		return Ev::lift();
	});
}

}

#endif /* !defined(EV_FOREACH_HPP) */
