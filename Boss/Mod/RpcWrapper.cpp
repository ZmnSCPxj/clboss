#include"Boss/Mod/Rpc.hpp"
#include"Boss/Mod/RpcWrapper.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/RequestRpcCommand.hpp"
#include"Boss/Msg/ResponseRpcCommand.hpp"
#include"Boss/concurrent.hpp"
#include"Ev/foreach.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<vector>

namespace Boss { namespace Mod {

class RpcWrapper::Impl {
private:
	S::Bus& bus;

	/*~ The core object that will actually handle
	 * the command.
	 */
	Boss::Mod::Rpc* rpc;
	/*~ Any pending requests that were received
	 * before `Boss::Msg::Init` was.
	 */
	std::vector<Boss::Msg::RequestRpcCommand> pending;

	void start() {
		rpc = nullptr;

		bus.subscribe<Msg::Init
			     >([this](Msg::Init const& init) {
			rpc = &init.rpc;

			auto f = [ this
				 ](Msg::RequestRpcCommand req) {
				return handle_command(req);
			};
			/* Grab the contents of pending, to
			 * ensure that no storage for it
			 * remains allocated.
			 */
			return Boss::concurrent(
				Ev::foreach(f, std::move(pending))
			);
		});

		bus.subscribe<Msg::RequestRpcCommand
			     >([this](Msg::RequestRpcCommand const& req) {
			return Boss::concurrent(handle_command(req));
		});
	}

	Ev::Io<void> handle_command(Msg::RequestRpcCommand const& req) {
		if (!rpc) {
			pending.push_back(req);
			return Ev::lift();
		}

		auto requester = req.requester;
		auto command = std::make_shared<std::string>(req.command);
		auto params = std::make_shared<Json::Out>(req.params);

		return Ev::lift().then([ this
				       , requester
				       , command
				       , params
				       ]() {
			return rpc->command( *command
					   , std::move(*params)
					   ).then([ this
						  , requester
						  ](Jsmn::Object result) {
				return bus.raise(Msg::ResponseRpcCommand{
					requester, true, std::move(result), ""
				});
			}).catching<RpcError>([ this
					      , requester
					      ](RpcError const& e) {
				return bus.raise(Msg::ResponseRpcCommand{
					requester, false, e.error, e.command
				});
			});
		});
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	explicit
	Impl(S::Bus& bus_) : bus(bus_) { start(); }
};

RpcWrapper::RpcWrapper(RpcWrapper&&) =default;
RpcWrapper::~RpcWrapper() =default;

RpcWrapper::RpcWrapper(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
