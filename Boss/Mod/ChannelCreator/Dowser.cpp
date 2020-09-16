#include"Boss/Mod/ChannelCreator/Dowser.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Ev/Io.hpp"
#include"Ev/map.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include"Ln/Scid.hpp"
#include<assert.h>
#include<string>
#include<vector>

namespace Boss { namespace Mod { namespace ChannelCreator {

class Dowser::Impl : public std::enable_shared_from_this<Impl> {
private:
	Boss::Mod::Rpc& rpc;
	Ln::NodeId proposal;
	Ln::NodeId patron;

	std::vector<std::string> excludes;
	Ln::Amount amount;

public:
	Impl( Boss::Mod::Rpc& rpc_
	    , Ln::NodeId const& self
	    , Ln::NodeId const& proposal_
	    , Ln::NodeId const& patron_
	    ) : rpc(rpc_)
	      , proposal(proposal_)
	      , patron(patron_)
	      , amount(Ln::Amount::sat(0))
	      {
		/* Exclude ourself from routing.  */
		excludes.emplace_back(std::string(self));
	}

	Ev::Io<Ln::Amount> run() {
		auto self = shared_from_this();
		return self->core_run().then([self]() {
			return Ev::lift(self->amount);
		});
	}

private:
	struct RouteStep {
		Ln::Scid chan;
		Ln::NodeId node;
	};
	Ev::Io<void> core_run() {
		return Ev::yield().then([this]() {
			return getroute();
		}).then([this](std::vector<RouteStep> route) {
			if (route.empty())
				/* Nothing to do now.  */
				return Ev::lift();
			add_exclude(route);
			return get_capacity( std::move(route)
					   ).then([this](Ln::Amount amt) {
				/* Deduct 1.5% from the capacity, to
				 * factor in 1% reserve and 0.5%
				 * default `maxfeepercent`.
				 */
				amount += amt * 0.985;
				return core_run();
			});
		});
	}
	Ev::Io<std::vector<RouteStep>> getroute() {
		auto exc = get_excludes();
		auto params = Json::Out()
			.start_object()
				.field("id", std::string(patron))
				.field("fromid", std::string(proposal))
				.field("msatoshi"
				      , std::string("1msat")
				      )
				/* I never had a decent grasp of
				 * riskfactor.  */
				.field("riskfactor", 10.0)
				.field("exclude", exc)
				.field("fuzzpercent", 0.0)
				.field("maxhops", 7)
			.end_object()
			;
		return rpc.command( "getroute"
				  , std::move(params)
				  ).then([this](Jsmn::Object res) {
			auto rv = std::vector<RouteStep>();
			auto empty = std::vector<RouteStep>();

			/* Parse result.  */
			if (!res.is_object() || !res.has("route"))
				return Ev::lift(empty);
			auto route = res["route"];
			if (!route.is_array())
				return Ev::lift(empty);
			for (auto i = std::size_t(0); i < route.size(); ++i) {
				auto step = route[i];
				if (!step.is_object())
					return Ev::lift(empty);
				if (!step.has("id"))
					return Ev::lift(empty);
				if (!step.has("channel"))
					return Ev::lift(empty);

				auto id_j = step["id"];
				auto chan_j = step["channel"];
				if (!id_j.is_string())
					return Ev::lift(empty);
				auto id_s = std::string(id_j);
				if (!Ln::NodeId::valid_string(id_s))
					return Ev::lift(empty);
				auto id = Ln::NodeId(id_s);

				if (!chan_j.is_string())
					return Ev::lift(empty);
				auto chan_s = std::string(chan_j);
				if (!Ln::Scid::valid_string(chan_s))
					return Ev::lift(empty);
				auto chan = Ln::Scid(chan_s);

				rv.emplace_back(RouteStep{chan, id});
			}

			return Ev::lift(std::move(rv));
		}).catching<RpcError>([this](RpcError const& _) {
			/* Assume this is route-not-found.  */
			return Ev::lift(std::vector<RouteStep>());
		});
	}
	Json::Out get_excludes() {
		auto json = Json::Out();
		auto arr = json.start_array();
		for (auto const& e : excludes)
			arr.entry(e);
		arr.end_array();
		return json;
	}

	/* Given a route, adds an exclusion for the first part of
	 * the route.  */
	void add_exclude(std::vector<RouteStep> const& route) {
		assert(!route.empty());
		if (route.size() == 1) {
			/* Exclude both directions of the first channel.  */
			auto s = std::string(route[0].chan);
			excludes.emplace_back(s + "/1");
			excludes.emplace_back(s + "/0");
		} else {
			/* Exclude first node.  */
			excludes.emplace_back(std::string(route[0].node));
		}
	}
	/* Gets the capacity of a route.  */
	Ev::Io<Ln::Amount> get_capacity(std::vector<RouteStep> route) {
		assert(!route.empty());
		using std::placeholders::_1;
		return Ev::map( std::bind(&Impl::get_1_capacity, this, _1)
			      , route
			      ).then([this](std::vector<Ln::Amount> amounts) {
			/* Get the smallest amount.  */
			auto theoretical_capacity =
				*std::min_element( amounts.begin()
						 , amounts.end()
						 );
			/* Divide by number of hops + 1.  */
			auto plausible_capacity =
				theoretical_capacity / (amounts.size() + 1);
			return Ev::lift(plausible_capacity);
		});
	}
	/* Gets the capacity of a single route hop.  */
	Ev::Io<Ln::Amount> get_1_capacity(RouteStep const& step) {
		auto scid = std::string(step.chan);
		return rpc.command( "listchannels"
				  , Json::Out()
					.start_object()
						.field( "short_channel_id"
						      , scid
						      )
					.end_object()
				  ).then([this](Jsmn::Object res) {
			auto zero = Ln::Amount::sat(0);
			if (!res.is_object() || !res.has("channels"))
				return Ev::lift(zero);
			auto chans = res["channels"];
			if (!chans.is_array())
				return Ev::lift(zero);
			for (auto i = std::size_t(0); i < chans.size(); ++i) {
				auto chan = chans[i];
				if ( !chan.is_object()
				  || !chan.has("amount_msat")
				   )
					continue;
				auto amt_j = chan["amount_msat"];
				if (!amt_j.is_string())
					continue;
				auto amt_s = std::string(amt_j);
				return Ev::lift(Ln::Amount(amt_s));
			}

			/* The channel *can* disappear between the time
			 * we did `getroute` to the time we did
			 * `listchannels`, so if we reach here, treat
			 * this as 0-capacity.
			 */
			return Ev::lift(zero);
		});
	}
};

Dowser::Dowser( Boss::Mod::Rpc& rpc
	      , Ln::NodeId const& self
	      , Ln::NodeId const& proposal
	      , Ln::NodeId const& patron
	      ) : pimpl(std::make_shared<Impl>(rpc, self, proposal, patron))
		{ }

Ev::Io<Ln::Amount> Dowser::run() {
	return pimpl->run();
}

}}}
