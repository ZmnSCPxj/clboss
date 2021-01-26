#ifndef BOSS_MOD_COMPLAINERBYLOWCONNECTRATE_HPP
#define BOSS_MOD_COMPLAINERBYLOWCONNECTRATE_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ComplainerByLowConnectRate
 *
 * @brief Module to raise complaints on peers that have a low
 * three-day connect rate.
 */
class ComplainerByLowConnectRate {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	ComplainerByLowConnectRate() =delete;
	ComplainerByLowConnectRate(ComplainerByLowConnectRate const&) =delete;

	ComplainerByLowConnectRate(ComplainerByLowConnectRate&&);
	~ComplainerByLowConnectRate();
	explicit
	ComplainerByLowConnectRate(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_COMPLAINERBYLOWCONNECTRATE_HPP) */
