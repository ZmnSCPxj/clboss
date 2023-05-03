#include"Boss/Mod/OnchainFundsAnnouncer.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Block.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/OnchainFunds.hpp"
#include"Boss/Msg/RequestGetOnchainIgnoreFlag.hpp"
#include"Boss/Msg/ResponseGetOnchainIgnoreFlag.hpp"
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
		return get_ignore_rr.execute(Msg::RequestGetOnchainIgnoreFlag{
			nullptr
		});
	}).then([this](Msg::ResponseGetOnchainIgnoreFlag res) {
		if (res.ignore)
			return Boss::log( bus, Info
					, "OnchainFundsAnnouncer: "
					  "Ignoring onchain funds until "
					  "%f seconds from now."
					, res.seconds
					);
		return announce();
	});
}

Ev::Io<void> OnchainFundsAnnouncer::announce() {
	return Ev::lift().then([this]() {
		return fundpsbt();
	}).then([this](Jsmn::Object res) {
		if (!res.is_object())
			return fail("fundpsbt did not return object", res);
		if (!res.has("excess_msat"))
			return fail("fundpsbt has no excess_msat", res);
		auto excess_msat = res["excess_msat"];
		if (!Ln::Amount::valid_object(excess_msat))
			return fail( "fundpsbt excess_msat not a valid amount"
				   , excess_msat
				   );
		auto amount = Ln::Amount::object(excess_msat);

		return Boss::log( bus, Debug
				, "OnchainFundsAnnouncer: "
				  "Found %s (after deducting fee to spend) onchain."
				, std::string(amount).c_str()
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

Ev::Io<Jsmn::Object>
OnchainFundsAnnouncer::fundpsbt() {
	return Ev::lift().then([this]() {
		/* On old C-Lightning, "reserve" is a bool.
		 * Try that first.
		 * If we get a parameter error -32602,
		 * try with a number 0.
		 */
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
	}).catching<RpcError>([this](RpcError const& e) {
		/* If not a parameter error, throw.  */
		if (int(double(e.error["code"])) != -32602) {
			throw e;
		}
		/* Retry with "reserve" as a number.  */
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
				.field("reserve", (double) 0)
			.end_object()
			;
		return rpc->command("fundpsbt", std::move(params));
	});
}

Ev::Io<void>
OnchainFundsAnnouncer::fail( std::string const& msg
			   , Jsmn::Object res
			   ) {
	auto os = std::ostringstream();
	os << res;
	return Boss::log( bus, Error
			, "OnchainFundsAnnouncer: %s: %s"
			, msg.c_str()
			, os.str().c_str()
			);
}

OnchainFundsAnnouncer::~OnchainFundsAnnouncer() =default;
OnchainFundsAnnouncer::OnchainFundsAnnouncer(S::Bus& bus_)
	: bus(bus_), rpc(nullptr)
	, get_ignore_rr(bus_)
	{ start(); }

}}

