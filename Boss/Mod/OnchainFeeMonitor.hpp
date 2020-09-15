#ifndef BOSS_MOD_ONCHAINFEEMONITOR_HPP
#define BOSS_MOD_ONCHAINFEEMONITOR_HPP

#include<memory>

namespace Boss { namespace Mod { class Waiter; }}
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::OnchainFeeMonitor
 *
 * @brief module to keep track of onchain fees.
 *
 * @desc keeps track of historical average (mean)
 * of fees, storing it in a table in the database.
 * Announces changes between "high fee regime" and
 * "low fee regime", and can be queried as well for
 * whether we are in low-fee or high-fee regime.
 */
class OnchainFeeMonitor {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	OnchainFeeMonitor() =delete;
	OnchainFeeMonitor(OnchainFeeMonitor const&) =delete;
	OnchainFeeMonitor(S::Bus&, Boss::Mod::Waiter&);
	OnchainFeeMonitor(OnchainFeeMonitor&&);
	~OnchainFeeMonitor();

	bool is_low_fee() const;
	bool is_high_fee() const { return !is_low_fee(); }
};

}}

#endif /* !defined(BOSS_MOD_ONCHAINFEEMONITOR_HPP) */
