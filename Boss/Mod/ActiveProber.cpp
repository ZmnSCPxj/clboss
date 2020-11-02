#include"Boss/Mod/ActiveProber.hpp"
#include"Boss/Mod/ChannelCandidateInvestigator/Main.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ProbeActively.hpp"
#include"Boss/concurrent.hpp"
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
#include"S/Bus.hpp"
#include"Sha256/Hash.hpp"
#include"Util/stringify.hpp"
#include<algorithm>
#include<memory>
#include<queue>
#include<set>
#include<vector>

namespace {

/* Try to probe the following amount.  */
auto const reference_amount = Ln::Amount::sat(160000);

Ev::Io<void> wait_for_rpc(Boss::Mod::Rpc*& rpc) {
	return Ev::yield().then([&rpc]() {
		if (!rpc)
			return wait_for_rpc(rpc);
		return Ev::lift();
	});
}

}

namespace Boss { namespace Mod {

class ActiveProber::Run : public std::enable_shared_from_this<Run> {
private:
	S::Bus& bus;
	ChannelCandidateInvestigator::Main& investigator;
	Rpc& rpc;
	Ln::NodeId self_id;

	Ln::NodeId peer;

	Secp256k1::Random& random;

	Run( ActiveProber& prober
	   , Ln::NodeId const& peer_
	   ) : bus(prober.bus)
	     , investigator(prober.investigator)
	     , rpc(*prober.rpc)
	     , self_id(prober.self_id)
	     , peer(peer_)
	     , random(prober.random)
	     { }

public:
	static
	std::shared_ptr<Run>
	create( ActiveProber& prober
	      , Ln::NodeId const& peer
	      ) {
		return std::shared_ptr<Run>(new Run(prober, peer));
	}

	Ev::Io<void> run() {
		auto self = shared_from_this();
		return self->core_run().then([self]() {
			return Ev::lift();
		});
	}
private:
	/* First channel.  */
	Ln::Scid chan0;
	Ln::Amount cap0;
	Ln::Amount amount;

	Ev::Io<void> core_run() {
		return Ev::lift().then([this]() {
			return Boss::log( bus, Info
					, "ActiveProber: Probe peer %s."
					, std::string(peer).c_str()
					);
		}).then([this]() {
			auto parms = Json::Out()
				.start_object()
					.field("id", std::string(peer))
				.end_object()
				;
			return rpc.command("listpeers", std::move(parms));
		}).then([this](Jsmn::Object res) {
			try {
				auto ps = res["peers"];
				for ( auto i = std::size_t(0)
				    ; i < ps.size()
				    ; ++i
				    ) {
					auto p = ps[i];
					auto cs = p["channels"];
					for ( auto j = std::size_t(0)
					    ; j < cs.size()
					    ; ++j
					    ) {
						auto c = cs[j];
						if (!c.has("short_channel_id"))
							continue;
						if (!c.has("spendable_msat"))
							continue;
						auto state = std::string(
							c["state"]
						);
						if (state != "CHANNELD_NORMAL")
							continue;

						chan0 = Ln::Scid(std::string(
							c["short_channel_id"]
						));
						cap0 = Ln::Amount(std::string(
							c["spendable_msat"]
						));
						break;
					}

					if (chan0)
						break;
				}
			} catch (Jsmn::TypeError const& _) {
				return Boss::log( bus, Error
						, "ActiveProber: unexpected "
						  "listpeers result: %s"
						, Util::stringify(res).c_str()
						);
			}

			if (!chan0)
				return Boss::log( bus, Info
						, "ActiveProber: No "
						  "CHANNELD_NORMAL channel "
						  "with node %s, cannot probe."
						, std::string(peer).c_str()
						);

			amount = reference_amount;
			if (amount > cap0 * 0.95)
				amount = cap0 * 0.95;

			return get_destinations();
		});
	}

	/* A candidate set of destinations.  */
	std::queue<Ln::NodeId> to_try;

	Ev::Io<void> get_destinations() {
		return Ev::lift().then([this]() {
			return investigator.get_channel_candidates();
		}).then([this](std::vector<std::pair< Ln::NodeId
						    , Ln::NodeId
						    >> cands) {
			/* Put both proposal and patron into the
			 * set of candidate nodes.  */
			auto n_set = std::set<Ln::NodeId>();
			for (auto& e : cands) {
				n_set.insert(std::move(e.first));
				n_set.insert(std::move(e.second));
			}
			/* If the peer itself is in the set, remove it.  */
			auto it = n_set.find(peer);
			if (it != n_set.end())
				n_set.erase(it);
			/* Copy to a vector and shuffle.  */
			auto n_vec = std::vector<Ln::NodeId>(n_set.size());
			std::copy( n_set.begin(), n_set.end()
				 , n_vec.begin()
				 );
			std::shuffle( n_vec.begin(), n_vec.end()
				    , Boss::random_engine
				    );
			/* Push to queue.  */
			for (auto& n : n_vec)
				to_try.push(std::move(n));

			if (to_try.empty())
				return Boss::log( bus, Info
						, "ActiveProber: No trial "
						  "destinations found "
						  "for peer %s."
						, std::string(peer).c_str()
						);

			return Boss::log( bus, Debug
					, "ActiveProber: Found %zu trial "
					  "destinations for peer %s."
					, to_try.size()
					, std::string(peer).c_str()
					)
			     + getroute()
			     ;
		});
	}

	Ln::Scid chan1;
	Ln::Amount amount1;
	std::uint32_t delay1;
	/* Route except for the 0th hop from us to peer.  */
	Jsmn::Object route;

	Ev::Io<void> getroute() {
		if (to_try.empty())
			return Boss::log( bus, Info
					, "ActiveProber: No more trial "
					  "destinations for peer %s."
					, std::string(peer).c_str()
					);
		return Ev::yield().then([this]() {
			auto const& dest = to_try.front();

			auto parms = Json::Out()
				.start_object()
					.field("fromid", std::string(peer))
					.field("id", std::string(dest))
					.field( "msatoshi"
					      , std::string(amount)
					      )
					/* I have written this many times,
					 * but I never understood riskfactor.
					 */
					.field("riskfactor", 10)
					.field("fuzzpercent", 95)
					.field("cltv", 14)
					.start_array("exclude")
						.entry(std::string(self_id))
					.end_array()
				.end_object()
				;
			return rpc.command("getroute", std::move(parms));
		}).then([this](Jsmn::Object res) {
			try {
				route = res["route"];
				auto hop1 = route[0];
				chan1 = Ln::Scid(std::string(
					hop1["channel"]
				));
				amount1 = Ln::Amount(std::string(
					hop1["amount_msat"]
				));
				delay1 = std::uint32_t(double(
					hop1["delay"]
				));
			} catch (Jsmn::TypeError const& _) {
				return Boss::log( bus, Error
						, "ActiveProber: Unexpected "
						  "result from getroute: %s"
						, Util::stringify(res).c_str()
						).then([]() {
					return Ev::lift(false);
				});
			}

			return Ev::lift(true);
		}).catching<RpcError>([this](RpcError const& e) {
			/* Go to next.  */
			to_try.pop();
			return Ev::lift(false);
		}).then([this](bool flag) {
			if (flag)
				return compute_hop0();
			else
				return getroute();
		});
	}

	/* Details from the first hop in the found route, which have to be
	 * added in the 0th hop we will insert.
	 */
	Ln::Amount base_fee;
	std::uint32_t proportional_fee;
	std::uint32_t cltv_delta;
	Ln::Amount amount0;
	std::uint32_t delay0;

	Ev::Io<void> compute_hop0() {
		return Ev::lift().then([this]() {
			/* Get information on chan1.  */
			auto parms = Json::Out()
				.start_object()
					.field( "short_channel_id"
					      , std::string(chan1)
					      )
				.end_object()
				;
			return rpc.command("listchannels", std::move(parms));
		}).then([this](Jsmn::Object res) {
			auto found = false;
			try {
				auto cs = res["channels"];
				for ( auto i = std::size_t(0)
				    ; i < cs.size()
				    ; ++i
				    ) {
					auto c = cs[i];
					auto src = Ln::NodeId(std::string(
						c["source"]
					));
					if (src != peer)
						continue;
					base_fee = Ln::Amount::msat(double(
						c["base_fee_millisatoshi"]
					));
					proportional_fee
						= std::uint32_t(double(
						c["fee_per_millionth"]
					));
					cltv_delta = std::uint32_t(double(
						c["delay"]
					));
					found = true;
				}
			} catch (Jsmn::TypeError const&) {
				return Boss::log( bus, Error
						, "ActiveProber: Unexpected "
						  "result from listchannels: "
						  "%s"
						, Util::stringify(res).c_str()
						);
			}

			/* Channel could have disappeared from under us.  */
			if (!found) {
				to_try.pop();
				return getroute();
			}

			amount0 = amount1 + base_fee
				+ (amount1 * ( double(proportional_fee)
					     / 1000000
					     ))
				/* Fudge roundoff erors.  */
				+ Ln::Amount::msat(1)
				;
			delay0 = delay1 + cltv_delta;

			return select_hash();
		});
	}

	Sha256::Hash hash;

	Ev::Io<void> select_hash() {
		/* Generate a random preimage and hash it.  */
		auto preimage = Ln::Preimage(random);
		hash = preimage.sha256();

		/* Flip the bits of the generated hash.
		 * This ensures that even if the entropy of our preimage
		 * is low, this is still not exploitable, as an attacker
		 * that knows every bit of our preimage still cannot
		 * reverse the inverse-hash of our preimage.
		 */
		std::uint8_t buf[32];
		hash.to_buffer(buf);
		for (auto i = std::size_t(0); i < 32; ++i)
			buf[i] = ~buf[i];
		hash.from_buffer(buf);

		return sendpay();
	}

	Ev::Io<void> sendpay() {
		return Ev::lift().then([this]() {
			auto os = std::ostringstream();
			os << chan0;
			for (auto i = std::size_t(0); i < route.size(); ++i)
				os << " " << std::string(route[i]["channel"]);
			return Boss::log( bus, Debug
					, "ActiveProber: Probe %s by route %s."
					, std::string(peer).c_str()
					, os.str().c_str()
					);
		}).then([this]() {
			auto routeparm = Json::Out();
			auto routearr = routeparm.start_array();
			/* Load the 0th hop.  */
			routearr.start_object()
					.field("id", std::string(peer))
					.field("channel", std::string(chan0))
					.field( "direction"
					      , self_id > peer ? 1 : 0
					      )
					.field( "msatoshi"
					      , amount0.to_msat()
					      )
					.field( "amount_msat"
					      , std::string(amount0)
					      )
					.field("delay", delay0)
					.field("style", "tlv")
				.end_object();
			/* Load the rest of the hops.  */
			for ( auto i = std::size_t(0)
			    ; i < route.size()
			    ; ++i
			    )
				routearr.entry(route[i]);
			routearr.end_array();

			auto label = std::string( "CLBOSS ActiveProber "
						  "payment, this will fail "
						  "and should automatically "
						  "get deleted. Hash: "
						)
				   + std::string(hash)
				   ;

			auto parms = Json::Out()
				.start_object()
					.field("route", std::move(routeparm))
					.field( "payment_hash"
					      , std::string(hash)
					      )
					.field("label", label)
				.end_object()
				;
			return rpc.command("sendpay", std::move(parms));
		}).then([this](Jsmn::Object _) {

			auto parms = Json::Out()
				.start_object()
					.field( "payment_hash"
					      , std::string(hash)
					      )
				.end_object()
				;
			return rpc.command("waitsendpay", std::move(parms));
		}).then([](Jsmn::Object _) {

			/* Oh look, we succeeded.
			 * Should not happen though.
			 */
			return Ev::lift(true);
		}).catching<RpcError>([](RpcError const& _) {
			/* Oh no, we failed, as expected.  */
			return Ev::lift(false);
		}).then([this](bool success) {

			auto status = std::string(
				success ? "complete" : "failed"
			);
			/* Now delete the payment, so that the operator
			 * does not get confused with random failing
			 * payments they did not make.  */
			auto parms = Json::Out()
				.start_object()
					.field( "payment_hash"
					      , std::string(hash)
					      )
					.field( "status"
					      , status
					      )
				.end_object()
				;
			return rpc.command("delpay", std::move(parms));
		}).then([](Jsmn::Object _) {
			/* We do not actually care if the `delpay` succeeds
			 * or not.
			 */
			return Ev::lift();
		}).catching<RpcError>([](RpcError const& _) {
			return Ev::lift();
		}).then([this]() {
			return Boss::log( bus, Info
					, "ActiveProber: Finished probing "
					  "peer %s."
					, std::string(peer).c_str()
					);
		});
	}
};

void ActiveProber::start() {
	bus.subscribe<Msg::Init
		     >([this](Msg::Init const& init) {
		rpc = &init.rpc;
		self_id = init.self_id;
		return Ev::lift();
	});
	bus.subscribe<Msg::ProbeActively
		     >([this](Msg::ProbeActively const& m) {
		auto run = Run::create(*this, m.peer);
		return Boss::concurrent( wait_for_rpc(rpc)
				       + run->run()
				       );
	});
}

}}
