#ifndef BOSS_MOD_INITIATOR_HPP
#define BOSS_MOD_INITIATOR_HPP

#include<functional>
#include<memory>
#include<string>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { class ThreadPool; }
namespace Net { class Fd; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::Initiator
 *
 * @brief handles the `init` command, creates the RPC
 * interface and broadcasts the Boss::Msg::Init
 * message which contains the RPC.
 */
class Initiator {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Initiator() =delete;

	Initiator( S::Bus& bus
		 , Ev::ThreadPool& threadpool
		 , std::function<Net::Fd( std::string const&
					, std::string const&
					)> open_rpc_socket
		 );
	Initiator(Initiator&&);
	~Initiator();
};

}}

#endif /* !defined(BOSS_MOD_INITIATOR_HPP) */
