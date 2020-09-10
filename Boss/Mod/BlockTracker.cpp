#include"Boss/Mod/BlockTracker.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Block.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include<inttypes.h>

namespace Boss { namespace Mod {

void BlockTracker::start() {
	height = 0;
	bus.subscribe<Boss::Msg::Init>([this](Boss::Msg::Init const& init) {
		/* Run this in the background (concurrent), we don't want to
		 * hold up the initialization!
		 */
		return Boss::concurrent(loop(init.rpc));
	});
}
Ev::Io<void> BlockTracker::loop(Boss::Mod::Rpc& rpc) {
	return Ev::yield().then([this, &rpc]() {
		auto js = Json::Out()
			.start_object()
				.field("blockheight", double(height + 1))
				.field("timeout", double(86400))
			.end_object()
			;
		return rpc.command("waitblockheight", js);
	}).then([this, &rpc] (Jsmn::Object result) {
		if (!result.is_object())
			return fail("waitblockheight: result not object.");
		if (!result.has("blockheight"))
			return fail("waitblockheight: no blockheight.");
		auto blockheight_js = result["blockheight"];
		if (!blockheight_js.is_number())
			return fail("waitblockheight: blockheight "
				    "not number."
				   );

		height = std::uint32_t(double(blockheight_js));
		return Ev::lift(true);
	}).catching<RpcError>([](RpcError const& e) {
		/* RPC Errors here are due to timeout.  Ignore and retry.
		 * Note that we need to catch this here since S::Bus::raise
		 * calls subscribers synchronously, meaning any errors raised
		 * by subscribers will propagate upwards.
		 * We want to specifically ignore failures of waitblockheight
		 * only; RPC errors from subscribers of the Block message
		 * should propagate upwards until they cause this entire
		 * greenthread to fail and terminate CLBOSS.
		 */
		return Ev::lift(false);
	}).then([this, &rpc](bool new_height) {
		if (!new_height)
			/* Timed out, just loop here.  */
			return Ev::lift();;
		return Ev::lift().then([this]() {
			return Boss::log( bus, Info
					, "New block at %" PRIu32
					, height
					);
		}).then([this]() {
			return bus.raise(Boss::Msg::Block{height});
		});
	}).then([this, &rpc]() {
		return loop(rpc);
	});
}

Ev::Io<bool> BlockTracker::fail(std::string f) {
	return Boss::log( bus, Error
			, "BlockTracker: %s"
			, f.c_str()
			).then([f]() {
		return Ev::Io<bool>([ f
				    ]( std::function<void(bool)> pass
				     , std::function<void(std::exception_ptr) > fail
				     ) {
			pass = nullptr;
			try {
				throw std::runtime_error(
					std::string("BlockTracker: ") +
					f
				);
			} catch (...) {
				fail(std::current_exception());
			}
		});
	});
}

}}
