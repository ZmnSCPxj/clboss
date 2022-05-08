#ifndef BOSS_MOD_REBALANCEUNMANAGER_HPP
#define BOSS_MOD_REBALANCEUNMANAGER_HPP

#include<memory>
#include<string>
#include<vector>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::RebalanceUnmanager
 *
 * @brief Provides a `balance` unmanagement tag, and
 * an interface to query the current set of nodes
 * that are banned from unmanagement.
 */
class RebalanceUnmanager {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	RebalanceUnmanager() =delete;
	RebalanceUnmanager(RebalanceUnmanager const&) =delete;

	RebalanceUnmanager(RebalanceUnmanager&&);
	~RebalanceUnmanager();

	/* The second argument here is intended for mocking
	 * in tests.
	 */
	explicit
	RebalanceUnmanager( S::Bus&
			  , std::vector<std::string> unmanaged
				= std::vector<std::string>()
			  );
};

}}

#endif /* !defined(BOSS_MOD_REBALANCEUNMANAGER_HPP) */
