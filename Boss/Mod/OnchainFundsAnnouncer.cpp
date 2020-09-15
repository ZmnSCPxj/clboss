#include"Boss/Mod/OnchainFundsAnnouncer.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Block.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/OnchainFunds.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include<cstddef>
#include<sstream>

namespace {

/** minconf
 *
 * @brief Number of confirmations before we consider
 * the funds to be safe to be spent.
 *
 * @desc We have to be wary of double-spending attacks
 * mounted on us by attackers who take advantage of
 * blocks getting orphaned.
 * 3 seems safe without taking too long.
 */
auto const minconf = std::size_t(3);

auto const startweight = std::size_t
( 42/* common*/
+ (8/*amount*/ + 1/*scriptlen*/ + 1/*push 0*/ + 1/*push*/ + 32/*p2wsh*/) * 4
);

}

namespace Boss { namespace Mod {

void OnchainFundsAnnouncer::start() {
	bus.subscribe< Msg::Init
		     >([this](Msg::Init const& init) {
		rpc = &init.rpc;
		return Ev::lift();
	});
	bus.subscribe< Msg::Block
		     >([this](Msg::Block const& _) {
		if (!rpc)
			return Ev::lift();
		return Boss::concurrent(on_block());
	});
}

Ev::Io<void> OnchainFundsAnnouncer::on_block() {
	return Ev::lift().then([this]() {
		auto params = Json::Out()
			.start_object()
				/* Get all the funds.  */
				.field("satoshi", std::string("all"))
				.field("feerate", std::string("normal"))
				.field("startweight", (double) startweight)
				.field("minconf", (double) minconf)
				/* Do not reserve; we just want to know
				 * how much money could be spent.
				 */
				.field("reserve", false)
			.end_object()
			;
		return rpc->command("fundpsbt", std::move(params));
	}).then([this](Jsmn::Object res) {
		if (!res.is_object())
			return fail("fundpsbt did not return object", res);
		if (!res.has("excess_msat"))
			return fail("fundpsbt has no excess_msat", res);
		auto excess_msat = res["excess_msat"];
		if (!excess_msat.is_string())
			return fail( "fundpsbt excess_msat not a string"
				   , excess_msat
				   );
		auto excess_msat_s = std::string(excess_msat);
		if (!Ln::Amount::valid_string(excess_msat_s))
			return fail( "fundpsbt excess_msat not valid amount"
				   , excess_msat
				   );
		auto amount = Ln::Amount(excess_msat_s);

		return Boss::log( bus, Debug
				, "OnchainFundsAnnouncer: "
				  "Found %s (after deducting fee to spend) onchain."
				, excess_msat_s.c_str()
				).then([this, amount]() {
			return bus.raise(Msg::OnchainFunds{amount});
		});
	}).catching<RpcError>([this](RpcError const& e) {
		return Boss::log( bus, Debug
				, "OnchainFundsAnnouncer: "
				  "No onchain funds found."
				);
	});
}

Ev::Io<void>
OnchainFundsAnnouncer::fail( std::string const& msg
			   , Jsmn::Object res
			   ) {
	auto os = std::ostringstream();
	os <<res;
	return Boss::log( bus, Error
			, "OnchainFundsAnnouncer: %s: %s"
			, msg.c_str()
			, os.str().c_str()
			);
}

}}

