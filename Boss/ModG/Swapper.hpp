#ifndef BOSS_MODG_SWAPPER_HPP
#define BOSS_MODG_SWAPPER_HPP

#include<functional>
#include<memory>
#include<string>

namespace Ev { template<typename a> class Io; }
namespace Ln { class Amount; }
namespace S { class Bus; }

namespace Boss { namespace ModG {

/** class Boss::ModG::Swapper
 *
 * @brief generic base for swapper modules.
 *
 * @desc swapper modules should only have
 * one swap in-flight at a time, and should
 * avoid swapping too eagerly.
 *
 * Derived classes determine the policy on when
 * to trigger swaps.
 */
class Swapper {
protected:
	/* Let derived classes access this for convenience.  */
	S::Bus& bus;

private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

protected:
	Swapper() =delete;
	Swapper(Swapper&&) =delete;
	Swapper(Swapper const&) =delete;

	~Swapper();

	explicit
	Swapper( S::Bus& bus
	       /* Should be constant in the derived
		* class.
		* This is used in database tables.
		*/
	       , char const* name
	       /* Used as the key in `clboss-status`.
		*/
	       , char const* statuskey
	       );

	/** Boss::ModG::Swapper::trigger_swap
	 *
	 * @brief Called by the derived class to check if
	 * it is OK to swap now.
	 *
	 * @desc the generic swapper will check if we are
	 * currently in, or just recently did, a swap.
	 * If so, this function will return immediately
	 * without any log messages.
	 *
	 * It will also check if the operator currently is
	 * telling us to ignore onchain, in which case we
	 * also return with a log messages.
	 *
	 * If not, it will call the given function.
	 * The given function should return `false` if the
	 * swap should not push through after all.
	 * It should return `true` if the swap should
	 * indeed push through, and set the `Ln::Amount&`
	 * argument to the amount to request.
	 *
	 * In either case the function should set the
	 * `std::string&` with a human-readable reason.
	 */
	Ev::Io<void>
	trigger_swap(std::function<bool(Ln::Amount&, std::string&)>);
};

}}

#endif /* !defined(BOSS_MODG_SWAPPER_HPP) */
