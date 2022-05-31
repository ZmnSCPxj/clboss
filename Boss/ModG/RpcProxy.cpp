#include"Boss/Mod/Rpc.hpp"
#include"Boss/ModG/RpcProxy.hpp"
#include"Boss/Msg/RequestRpcCommand.hpp"
#include"Boss/Msg/ResponseRpcCommand.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"

namespace Boss { namespace ModG {

RpcProxy::~RpcProxy() =default;

RpcProxy::RpcProxy(S::Bus& bus)
	: core(bus) { }

Ev::Io<Jsmn::Object>
RpcProxy::command( std::string const& command
		 , Json::Out params
		 ) {
	return core.execute(Msg::RequestRpcCommand{
		nullptr, command, params
	}).then([](Msg::ResponseRpcCommand resp) {
		if (!resp.succeeded)
			throw Mod::RpcError( std::move(resp.command)
					   , std::move(resp.result)
					   );
		return Ev::lift(std::move(resp.result));
	});
}

}}
