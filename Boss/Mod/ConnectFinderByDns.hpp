#ifndef BOSS_MOD_CONNECTFINDERBYDNS_HPP
#define BOSS_MOD_CONNECTFINDERBYDNS_HPP

#include<memory>

namespace S { class Bus; }
namespace Ev { class ThreadPool; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ConnectFinderByDns
 *
 * @brief module to find new connection candidates
 * by referring to DNS seeds.
 */
class ConnectFinderByDns {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	ConnectFinderByDns() =delete;
	explicit
	ConnectFinderByDns(S::Bus& bus, Ev::ThreadPool& threadpool);
	ConnectFinderByDns(ConnectFinderByDns&&);
	~ConnectFinderByDns();
};

}}

#endif /* !defined(BOSS_MOD_CONNECTFINDERBYDNS_HPP) */
