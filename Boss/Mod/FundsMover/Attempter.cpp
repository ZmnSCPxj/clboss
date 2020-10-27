#include"Boss/Mod/FundsMover/Attempter.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/log.hpp"
#include"Boss/random_engine.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include"Ln/Preimage.hpp"
#include"Ln/Scid.hpp"
#include"Sha256/Hash.hpp"
#include"Util/stringify.hpp"
#include<random>
#include<string>
#include<vector>

namespace Boss { namespace Mod { namespace FundsMover {

class Attempter::Impl : public std::enable_shared_from_this<Impl> {
private:
	S::Bus& bus;

	Boss::Mod::Rpc& rpc;
	Ln::NodeId self_id;
	Ln::Preimage preimage;
	Ln::NodeId source;
	Ln::NodeId destination;
	Ln::Amount amount;
	std::shared_ptr<Ln::Amount> fee_budget;
	/* Details of the last channel from destination to us.  */
	Ln::Scid last_scid;
	Ln::Amount base_fee;
	std::uint32_t proportional_fee;
	std::uint32_t cltv_delta;
	/* Details of the first channel from us to source.  */
	Ln::Scid first_scid;

	bool ok;

	double fuzzpercent;
	std::vector<std::string> excludes;
	Ln::Amount dest_amount;
	Ln::Amount source_amount;
	std::uint32_t source_delay;

	/* Route from source to destination, not including the
	 * us->source and destination->us hops.
	 */
	Jsmn::Object route;

	/* The fee we currently have.  */
	Ln::Amount our_fee;

public:
	Impl( S::Bus& bus_
	    , Boss::Mod::Rpc& rpc_
	    , Ln::NodeId self_id_
	    , Ln::Preimage preimage_
	    , Ln::NodeId source_
	    , Ln::NodeId destination_
	    , Ln::Amount amount_
	    , std::shared_ptr<Ln::Amount> fee_budget_
	    , Ln::Scid last_scid_
	    , Ln::Amount base_fee_
	    , std::uint32_t proportional_fee_
	    , std::uint32_t cltv_delta_
	    , Ln::Scid first_scid_
	    ) : bus(bus_)
	      , rpc(rpc_)
	      , self_id(std::move(self_id_))
	      , preimage(std::move(preimage_))
	      , source(std::move(source_))
	      , destination(std::move(destination_))
	      , amount(amount_)
	      , fee_budget(std::move(fee_budget_))
	      , last_scid(last_scid_)
	      , base_fee(base_fee_)
	      , proportional_fee(proportional_fee_)
	      , cltv_delta(cltv_delta_)
	      , first_scid(first_scid_)
	      , ok(false)
	      { }
	Ev::Io<bool> run() {
		auto self = shared_from_this();
		return self->core_run().then([self]() {
			return Ev::lift(self->ok);
		});
	}

private:
	Ev::Io<void> core_run() {
		return Ev::lift().then([this]() {
			/* Initialize.  */
			excludes.push_back(std::string(self_id));
			dest_amount = amount + base_fee
				    + (amount * ( double(proportional_fee)
						/ 1000000
						))
				    + Ln::Amount::msat(1)
				    ;
			fuzzpercent = 99.0;

			return getroute();
		});
	}
	Ev::Io<void> getroute() {
		return Ev::yield().then([this]() {
			auto parms = Json::Out()
				.start_object()
					.field("id", std::string(destination))
					.field( "msatoshi"
					      , std::string(dest_amount)
					      )
					.field("riskfactor", 10)
					.field("cltv", cltv_delta + 14)
					.field("fromid", std::string(source))
					.field("fuzzpercent", fuzzpercent)
				.end_object()
				;
			return rpc.command("getroute", std::move(parms));
		}).then([this](Jsmn::Object res) {
			if (!res.is_object() || !res.has("route"))
				return Ev::lift(false);
			route = res["route"];
			return Ev::lift(true);
		}).catching<RpcError>([](RpcError const&) {
			return Ev::lift(false);
		}).then([this](bool ok) {
			if (!ok)
				/* Maybe lowering the fuzzpercent will work
				 * this time?  */
				return fee_failed();
			return compute_source_amount();
		});
	}
	Ev::Io<void> compute_source_amount() {
		struct Fail { };
		auto hop1_amount = std::make_shared<Ln::Amount>();
		auto hop1_delay = std::make_shared<std::uint32_t>();

		return Ev::lift().then([this, hop1_amount, hop1_delay]() {
			auto hop1 = Ln::Scid();
			try {
				auto hop1_data = route[0];
				hop1 = Ln::Scid(std::string(
					hop1_data["channel"]
				));
				*hop1_amount = Ln::Amount(std::string(
					hop1_data["amount_msat"]
				));
				*hop1_delay = std::uint32_t(double(
					hop1_data["delay"]
				));
			} catch (Jsmn::TypeError const& ) {
				return Boss::log( bus, Error
						, "FundsMover: Unexpected "
						  "route from getroute: %s"
						, Util::stringify(route)
							.c_str()
						).then([this]() {
					throw Fail();
					return Ev::lift(Jsmn::Object());
				});
			}
			auto parms = Json::Out()
				.start_object()
					.field( "short_channel_id"
					      , std::string(hop1)
					      )
				.end_object()
				;
			return rpc.command("listchannels", std::move(parms));
		}).then([this, hop1_amount, hop1_delay](Jsmn::Object res) {
			auto base_fee = Ln::Amount();
			auto prop_fee = std::uint32_t();
			auto cltv_delta = std::uint32_t();
			try {
				auto found = false;
				auto cs = res["channels"];
				for ( auto i = std::size_t(0)
				    ; i < cs.size()
				    ; ++i
				    ) {
					auto c = cs[i];
					auto csrc = Ln::NodeId(std::string(
						c["source"]
					));
					if (csrc != source)
						continue;
					found = true;
					base_fee = Ln::Amount::msat(double(
						c["base_fee_millisatoshi"]
					));
					prop_fee = std::uint32_t(double(
						c["fee_per_millionth"]
					));
					cltv_delta = std::uint32_t(double(
						c["delay"]
					));
				}
				if (!found)
					throw Jsmn::TypeError();
			} catch (Jsmn::TypeError const&) {
				return Boss::log( bus, Error
						, "FundsMover: Unexpected "
						  "result from listchannels: "
						  "%s"
						, Util::stringify(res)
							.c_str()
						).then([this]() {
					return Ev::lift(false);
				});
			}
			source_amount = *hop1_amount + base_fee
				      + (*hop1_amount * ( double(prop_fee)
							/ 1000000
							))
				      + Ln::Amount::msat(1)
				      ;
			source_delay = *hop1_delay + cltv_delta;
			our_fee = source_amount - amount;
			if (our_fee > *fee_budget)
				return Ev::lift(false);
			*fee_budget -= our_fee;
			return Ev::lift(true);
		}).catching<RpcError>([](RpcError const&) {
			return Ev::lift(false);
		}).catching<Fail>([](Fail const&) {
			return Ev::lift(false);
		}).then([this](bool success) {
			if (!success)
				return fee_failed();
			/* At this point we have deducted our fee from the
			 * fee budget.
			 */
			return sendpay();
		});
	}

	Ev::Io<void> sendpay() {
		auto payment_hash = std::make_shared<Sha256::Hash>();
		return Ev::lift().then([this, payment_hash]() {
			*payment_hash = preimage.sha256();
			auto label = std::string("CLBOSS FundsMover payment, "
						 "this should automatically "
						 "get deleted. Hash: ")
				   + std::string(*payment_hash)
				   ;
			auto parms = Json::Out()
				.start_object()
					.field("route", make_route())
					.field( "payment_hash"
					      , std::string(*payment_hash)
					      )
					.field("label" , label)
				.end_object()
				;
			return rpc.command("sendpay", std::move(parms));
		}).then([this, payment_hash](Jsmn::Object _) {
			auto parms = Json::Out()
				.start_object()
					.field( "payment_hash"
					      , std::string(*payment_hash)
					      )
				.end_object()
				;
			return rpc.command("waitsendpay", std::move(parms));
		}).then([this, payment_hash](Jsmn::Object _) {
			/* Success?  Now we can stop.  */
			ok = true;
			return delpay(payment_hash, true)
			     + Boss::log( bus, Info
					, "FundsMover: Moved %s from %s, "
					  "getting %s to %s, costing us "
					  "%s."
					, std::string(source_amount).c_str()
					, std::string(source).c_str()
					, std::string(amount).c_str()
					, std::string(destination).c_str()
					, std::string(our_fee).c_str()
					)
			     ;
		}).catching<RpcError>([ this
				      , payment_hash
				      ](RpcError const& e) {
			/* Return our fee to the budget.  */
			*fee_budget += our_fee;

			/* Figure out the error.  */
			auto code = int();
			auto eidx = std::size_t();
			auto echan = Ln::Scid();
			auto edir = int();
			auto enode = Ln::NodeId();
			auto fail = std::uint16_t();
			try {
				auto& error = e.error;
				code = int(double(
					error["code"]
				));
				/* Failure along route.  */
				if (code == 204) {
					auto data = error["data"];
					eidx = std::size_t(double(
						data["erring_index"]
					));
					echan = Ln::Scid(std::string(
						data["erring_channel"]
					));
					edir = int(double(
						data["erring_direction"]
					));
					enode = Ln::NodeId(std::string(
						data["erring_node"]
					));
					fail = std::uint16_t(double(
						data["failcode"]
					));
				}
			} catch (std::exception const&) {
				return Boss::log( bus, Error
						, "FundsMover: Attempt: "
						  "Unexpected error from "
						  "%s: %s"
						, e.command.c_str()
						, Util::stringify(e.error)
							.c_str()
						);
			}

			if (code != 202 && code != 204)
				return Boss::log( bus, Info
						, "FundsMover: Attempt: "
						  "Unexpected error code "
						  "%d from %s, error: %s"
						, code
						, e.command
						, Util::stringify(e.error)
							.c_str()
						);
			/* Unparsable onion with a 1-hop route means the
			 * source or destination node has massive issues,
			 * so cannot advance.  */
			if (code == 202 && route.size() <= 1)
				return Boss::log( bus, Info
						, "FundsMover: Attempt: "
						  "Unparsable onion, cannot "
						  "advance further."
						);

			if (code == 204) {
				if ( eidx == 0
				  || (eidx == 1 && (fail & 0x2000))
				   )
					return Boss::log( bus, Info
							, "FundsMover: "
							  "Failed at source, "
							  "cannot advance "
							  "further."
							);
				if (eidx == route.size() + 1)
					return Boss::log( bus, Info
							, "FundsMover: "
							  "Failed at "
							  "destination, "
							  "cannot advance "
							  "further."
							);
				/* 0x2000 == NODE level error.  */
				if ((fail & 0x2000))
					excludes.push_back(std::string(
						enode
					));
				else
					excludes.push_back(
						std::string(echan) + "/" +
						Util::stringify(edir)
					);
			} else {
				/* Unparsable onion, exclude any node.  */
				auto dist = std::uniform_int_distribution<std::size_t>(
					0, route.size() - 2
				);
				auto hop = route[dist(Boss::random_engine)];
				excludes.push_back(std::string(
					hop["id"]
				));
			}

			return getroute();
		});
	}
	/* Splice the us->source and destination->us hops.  */
	Json::Out make_route() {
		auto ret = Json::Out();
		auto arr = ret.start_array();
		arr.entry(make_hop0());
		for (auto i = std::size_t(0); i < route.size(); ++i)
			arr.entry(route[i]);
		arr.entry(make_hoplast());
		arr.end_array();
		return ret;
	}
	Json::Out make_hop0() {
		/* Us->source hop.  */
		return Json::Out()
			.start_object()
				.field("id", std::string(source))
				.field("channel", std::string(first_scid))
				.field( "direction"
				      , self_id > source ? 1 : 0
				      )
				.field("msatoshi", source_amount.to_msat())
				.field( "amount_msat"
				      , std::string(source_amount)
				      )
				.field("delay", source_delay)
				/* Just to be sure.  */
				.field("style", "legacy")
			.end_object()
			;
	}
	Json::Out make_hoplast() {
		/* Destination->us hop.  */
		return Json::Out()
			.start_object()
				.field("id", std::string(self_id))
				.field("channel", std::string(last_scid))
				.field( "direction"
				      , destination > self_id ? 1 : 0
				      )
				.field("msatoshi", amount.to_msat())
				.field("amount_msat", std::string(amount))
				.field("delay", 14)
				.field("style", "tlv")
			.end_object()
			;
	}

	Ev::Io<void> delpay( std::shared_ptr<Sha256::Hash> payment_hash
			   , bool success
			   ) {
		return Ev::lift().then([this, payment_hash, success]() {
			auto status = std::string(
				success ? "complete" : "failed"
			);
			auto parms = Json::Out()
				.start_object()
					.field( "payment_hash"
					      , std::string(*payment_hash)
					      )
					.field("status", status)
				.end_object()
				;
			return rpc.command("delpay", std::move(parms));
			/* Do not care if it succeeds or fails or what
			 * it returns.  */
		}).then([](Jsmn::Object _) {
			return Ev::lift();
		}).catching<RpcError>([](RpcError const& _) {
			return Ev::lift();
		});
	}

	Ev::Io<void> fee_failed() {
		if (fuzzpercent > 0) {
			fuzzpercent -= 22;
			if (fuzzpercent < 0)
				fuzzpercent = 0;
			return getroute();
		}
		return Boss::log( bus, Debug
				, "FundsMover: Giving up attempt to move "
				  "%s from %s to %s."
				, std::string(amount).c_str()
				, std::string(source).c_str()
				, std::string(destination).c_str()
				);
	}
};

Ev::Io<bool>
Attempter::run( S::Bus& bus
	      , Boss::Mod::Rpc& rpc
	      , Ln::NodeId self
	      /* The preimage should have been pre-arranged to be claimed.  */
	      , Ln::Preimage preimage
	      , Ln::NodeId source
	      , Ln::NodeId destination
	      , Ln::Amount amount
	      , std::shared_ptr<Ln::Amount> fee_budget
	      /* Details of the channel from destination to us.  */
	      , Ln::Scid last_scid
	      , Ln::Amount base_fee
	      , std::uint32_t proportional_fee
	      , std::uint32_t cltv_delta
	      /* The channel from us to source.  */
	      , Ln::Scid first_scid
	      ) {
	auto impl = std::make_shared<Impl>( bus
					  , rpc
					  , std::move(self)
					  , std::move(preimage)
					  , std::move(source)
					  , std::move(destination)
					  , amount
					  , std::move(fee_budget)
					  , last_scid
					  , base_fee
					  , proportional_fee
					  , cltv_delta
					  , first_scid
					  );
	return impl->run();
}

}}}
