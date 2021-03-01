#ifndef BOSS_MOD_UNMANAGEDMANAGER_HPP
#define BOSS_MOD_UNMANAGEDMANAGER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::UnmanagedManager
 *
 * @brief Handles the set of unmanaged nodes.
 */
class UnmanagedManager {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	UnmanagedManager() =delete;

	UnmanagedManager(UnmanagedManager&&);
	~UnmanagedManager();

	explicit
	UnmanagedManager(S::Bus&);
};

}}

#endif /* !defined(BOSS_MOD_UNMANAGEDMANAGER_HPP) */
