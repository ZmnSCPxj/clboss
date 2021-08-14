#ifndef BOSS_MOD_PEERJUDGE_EXECUTIONER_HPP
#define BOSS_MOD_PEERJUDGE_EXECUTIONER_HPP

#include"Sqlite3/Db.hpp"
#include<functional>
#include<map>
#include<set>
#include<string>
#include<utility>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
namespace Ln { class NodeId; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace PeerJudge {

/** class Boss::Mod::PeerJudge::Executioner
 *
 * @brief Sub-module that handles actual closure
 * of channels that should be closed.
 * Also handles status of closed peers using key
 * `"closed_by_peer_judge"`.
 */
class Executioner {
private:
	S::Bus& bus;
	std::function<std::map<Ln::NodeId, std::string>()> get_close;
	std::function<std::set<Ln::NodeId> const&()> get_unmanaged;

	bool enabled;

	Sqlite3::Db db;
	Boss::Mod::Rpc* rpc;

	void start();
	Ev::Io<void> wait_init();

public:
	Executioner( S::Bus& bus_
		   , std::function< std::map<Ln::NodeId, std::string>()
			          > get_close_
		   , std::function< std::set<Ln::NodeId> const&()
				  > get_unmanaged_
		   ) : bus(bus_)
		     , get_close(std::move(get_close_))
		     , get_unmanaged(std::move(get_unmanaged_))
		     , db()
		     , rpc(nullptr)
		     { start(); }
	Executioner(Executioner const&) =delete;
	Executioner(Executioner&&) =delete;
};

}}}

#endif /* BOSS_MOD_PEERJUDGE_EXECUTIONER_HPP */
