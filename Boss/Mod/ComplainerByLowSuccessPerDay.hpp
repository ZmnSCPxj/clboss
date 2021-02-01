#ifndef BOSS_MOD_COMPLAINERBYLOWSUCCESSPERDAY_HPP
#define BOSS_MOD_COMPLAINERBYLOWSUCCESSPERDAY_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ComplainerByLowSuccessPerDay
 *
 * @brief Complains about peers that have a low `success_per_day`
 * metric, as presumably those peers have insufficient quality
 * in their connections.
 * This tries to up the metric as well by triggering active
 * probes for peers that are near the borderline.
 */
class ComplainerByLowSuccessPerDay {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	ComplainerByLowSuccessPerDay() =delete;

	~ComplainerByLowSuccessPerDay();
	ComplainerByLowSuccessPerDay(ComplainerByLowSuccessPerDay&&);

	explicit
	ComplainerByLowSuccessPerDay(S::Bus&);
};

}}

#endif /* !defined(BOSS_MOD_COMPLAINERBYLOWSUCCESSPERDAY_HPP) */
