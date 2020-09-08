#ifndef BOSS_MOD_RPC_HPP
#define BOSS_MOD_RPC_HPP

#include"Jsmn/Object.hpp"
#include<memory>
#include<stdexcept>
#include<string>

namespace Ev { template<typename a> class Io; }
namespace Json { class Out; }
namespace Net { class Fd; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

struct RpcError : public std::runtime_error {
private:
	static
	std::string make_error_message( std::string const&
				      , Jsmn::Object const&
				      );
public:
	RpcError() =delete;
	explicit
	RpcError(std::string command, Jsmn::Object error);

	std::string command;
	Jsmn::Object error;
};

/** class Boss::Mod::Rpc
 *
 * @brief module to provide access to a JSON-RPC
 * stream socket.
 *
 * @desc Module that handles a JSON-RPC stream.
 * This module is constructed later, after the
 * `init` method.
 */
class Rpc {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	explicit
	Rpc(S::Bus& bus, Net::Fd socket);
	Rpc(Rpc&&);
	~Rpc();

	Ev::Io<Jsmn::Object> command( std::string const& command
				    , Json::Out params
				    );
};

}}

#endif /* !defined(BOSS_MOD_RPC_HPP) */
