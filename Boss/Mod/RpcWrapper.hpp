#ifndef BOSS_MOD_RPCWRAPPER_HPP
#define BOSS_MOD_RPCWRAPPER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::RpcWrapper
 *
 * @brief Handles RPC commands sent via the bus.
 *
 * @desc This module allows RPC commands to be
 * sent via the bus, instead of the `Boss::Mod::Rpc`
 * object being sent over the `Boss::Msg::Init`
 * message.
 * By using the bus, it is easier to test modules
 * in isolation, since mocking the RPC.
 */
class RpcWrapper {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	RpcWrapper() =delete;
	RpcWrapper(RpcWrapper const&) =delete;

	RpcWrapper(RpcWrapper&&);
	~RpcWrapper();
	explicit
	RpcWrapper(S::Bus& bus);
};

}}

#endif /* BOSS_MOD_RPCWRAPPER_HPP */
