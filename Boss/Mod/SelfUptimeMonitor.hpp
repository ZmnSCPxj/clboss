#ifndef BOSS_MOD_SELFUPTIMEMONITOR_HPP
#define BOSS_MOD_SELFUPTIMEMONITOR_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::SelfUptimeMonitor
 *
 * @brief Monitors our own uptime.
 * Uptime is about being internet connected and CLBOSS is running.
 */
class SelfUptimeMonitor {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	SelfUptimeMonitor() =delete;

	SelfUptimeMonitor(SelfUptimeMonitor&&);
	~SelfUptimeMonitor();

	explicit
	SelfUptimeMonitor(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_SELFUPTIMEMONITOR_HPP) */
