#ifndef BOSS_MODG_RPCPROXY_HPP
#define BOSS_MODG_RPCPROXY_HPP

#include"Boss/ModG/ReqResp.hpp"
#include<string>

namespace Boss { namespace Msg { struct RequestRpcCommand; }}
namespace Boss { namespace Msg { struct ResponseRpcCommand; }}
namespace Ev { template<typename a> class Io; }
namespace Jsmn { class Object; }
namespace Json { class Out; }
namespace S { class Bus; }

namespace Boss { namespace ModG {

/** class Boss::ModG::RpcProxy
 *
 * @brief Utility object to act as the `Boss::Mod::Rpc`,
 * but utilizing the `S::Bus` underneath.
 */
class RpcProxy {
private:
	Boss::ModG::ReqResp<Msg::RequestRpcCommand, Msg::ResponseRpcCommand> core;

public:
	RpcProxy() =delete;
	~RpcProxy();

	explicit
	RpcProxy(S::Bus&);

	/*~ Perform the RPC, possibly throwing
	 * Boss::Mod::RpcError in case the
	 * C-Lightning RPC threw an error.
	 */
	Ev::Io<Jsmn::Object> command( std::string const& command
				    , Json::Out params
				    );
};

}}

#endif /* BOSS_MODG_RPCPROXY_HPP */
