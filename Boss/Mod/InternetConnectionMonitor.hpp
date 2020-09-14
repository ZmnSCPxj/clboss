#ifndef BOSS_MOD_INTERNETCONNECTIONMONITOR_HPP
#define BOSS_MOD_INTERNETCONNECTIONMONITOR_HPP

#include<memory>

namespace Boss { namespace Mod { class Waiter; }}
namespace Ev { class ThreadPool; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::InternetConnectionMonitor
 *
 * @brief Periodically checks that we can access the
 * Internet.
 * Provides a query method to determine if we currently
 * think we are disconnected from the Internet.
 */
class InternetConnectionMonitor {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	InternetConnectionMonitor() =delete;
	InternetConnectionMonitor(InternetConnectionMonitor const&) =delete;

	explicit
	InternetConnectionMonitor( S::Bus&
				 , Ev::ThreadPool&
				 , Boss::Mod::Waiter&
				 );
	InternetConnectionMonitor(InternetConnectionMonitor&&);
	~InternetConnectionMonitor();

	bool is_online() const;
};

}}

#endif /* !defined(BOSS_MOD_INTERNETCONNECTIONMONITOR_HPP) */
