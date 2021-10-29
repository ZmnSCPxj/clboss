#ifndef BOSS_MOD_SWAPREPORTER_HPP
#define BOSS_MOD_SWAPREPORTER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::SwapReporter
 *
 * @brief Monitors all `SwapManager` swaps and makes it available
 * on `clboss-status` as well as `clboss-swaps`.
 */
class SwapReporter {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	SwapReporter() =delete;
	SwapReporter(SwapReporter&&);
	~SwapReporter();

	explicit
	SwapReporter(S::Bus&);
};

}}

#endif /* !defined(BOSS_MOD_SWAPREPORTER_HPP) */
