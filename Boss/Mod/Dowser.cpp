#include"Boss/Mod/Dowser.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/CommandFail.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/ManifestCommand.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/RequestDowser.hpp"
#include"Boss/Msg/ResponseDowser.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/map.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Ln/Scid.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<assert.h>
#include<memory>
#include<string>
#include<vector>

namespace {

Ev::Io<void> wait_for_rpc(Boss::Mod::Rpc*& rpc) {
	return Ev::yield().then([&rpc]() {
		if (rpc)
			return Ev::lift();
		return wait_for_rpc(rpc);
	});
}

/* Maximum number of routes to try.  */
auto const dowser_limit = std::size_t(10);
/* Maximum length of routes.  */
auto const route_limit = std::size_t(3);

}

namespace Boss { namespace Mod {

class Dowser::Run : public std::enable_shared_from_this<Run> {
private:
	S::Bus& bus;
	void* requester;
	Ln::NodeId fromid;
	Ln::NodeId toid;

	Boss::Mod::Rpc* rpc;
	Ln::NodeId self_id;

	std::vector<std::string> excludes;
	Ln::Amount amount;
	std::size_t tries;

public:
	Run( S::Bus& bus_
	   , void* requester_
	   , Ln::NodeId const& fromid_
	   , Ln::NodeId const& toid_
	   ) : bus(bus_)
	     , requester(requester_)
	     , fromid(fromid_)
	     , toid(toid_)
	     { }

	Ev::Io<void> run( Boss::Mod::Rpc& rpc_
			, Ln::NodeId const& self_id_
			) {
		rpc = &rpc_;
		self_id = self_id_;
		auto self = shared_from_this();
		return Ev::lift().then([self]() {
			return self->core_run();
		}).then([self]() {
			return self->bus.raise(Msg::ResponseDowser{
				self->requester, self->amount
			});
		});
	}

private:
	Ev::Io<void> core_run() {
		return Ev::lift().then([this]() {
			amount = Ln::Amount::sat(0);
			tries = 0;
			excludes.emplace_back(std::string(self_id));
			return loop();
		});
	}
	struct RouteStep {
		Ln::Scid chan;
		Ln::NodeId node;
	};
	Ev::Io<void> loop() {
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
				++tries;
				if (tries >= dowser_limit)
					return Ev::lift();
				return loop();
			});
		});
	}
	Ev::Io<std::vector<RouteStep>> getroute() {
		auto exc = get_excludes();
		auto params = Json::Out()
			.start_object()
				.field("id", std::string(toid))
				.field("fromid", std::string(fromid))
				.field("msatoshi", "1msat")
				/* I never had a decent grasp of
				 * riskfactor.  */
				.field("riskfactor", 10.0)
				.field("exclude", exc)
				.field("fuzzpercent", 0.0)
				.field("maxhops", route_limit)
			.end_object()
			;
		return rpc->command( "getroute"
				   , std::move(params)
				   ).then([](Jsmn::Object res) {
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
		}).catching<RpcError>([](RpcError const& _) {
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
		return Ev::map( std::bind(&Run::get_1_capacity, this, _1)
			      , route
			      ).then([](std::vector<Ln::Amount> amounts) {
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
		return rpc->command( "listchannels"
				   , Json::Out()
					.start_object()
						.field( "short_channel_id"
						      , scid
						      )
					.end_object()
				   ).then([](Jsmn::Object res) {
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

class Dowser::CommandImpl {
private:
	S::Bus& bus;
	ModG::ReqResp< Msg::RequestDowser
		     , Msg::ResponseDowser
		     > dowse_rr;

	void start() {
		/* clboss-dowser command.  */
		bus.subscribe<Msg::Manifestation
			     >([this](Msg::Manifestation const& _) {
			return bus.raise(Msg::ManifestCommand{
				"clboss-dowser", "fromid toid",
				"Execute the dowsing algorithm to "
				"estimate the useful capacity between "
				"two nodes.",
				false
			});
		});
		bus.subscribe<Msg::CommandRequest
			     >([this](Msg::CommandRequest const& m) {
			if (m.command != "clboss-dowser")
				return Ev::lift();
			return run_command(m.params, m.id);
		});
	}

	Ev::Io<void> run_command(Jsmn::Object params, std::uint64_t id) {
		auto param_fail = [this, id]() {
			return bus.raise(Msg::CommandFail{
				id, -32602, "Parameter error",
				Json::Out::empty_object()
			});
		};
		auto fromid = Ln::NodeId();
		auto toid = Ln::NodeId();

		try {
			if (params.size() != 2)
				return param_fail();
			auto fromid_j = Jsmn::Object();
			auto toid_j = Jsmn::Object();
			if (params.is_object()) {
				fromid_j = params["fromid"];
				toid_j = params["toid"];
			} else if (params.is_array()) {
				fromid_j = params[0];
				toid_j = params[1];
			} else
				return param_fail();
			fromid = Ln::NodeId(std::string(fromid_j));
			toid = Ln::NodeId(std::string(toid_j));
		} catch (std::exception const&) {
			return param_fail();
		}

		return dowse_rr.execute(Msg::RequestDowser{
			nullptr, fromid, toid
		}).then([this, id](Msg::ResponseDowser r) {
			auto rsp = Json::Out()
				.start_object()
					.field("amount", std::string(r.amount))
				.end_object()
				;
			return bus.raise(Msg::CommandResponse{
				id, std::move(rsp)
			});
		});
	}

public:
	CommandImpl( S::Bus& bus_
		   ) : bus(bus_)
		     , dowse_rr( bus_
			       , [](Msg::RequestDowser& m, void* p) {
					m.requester = p;
				 }
			       , [](Msg::ResponseDowser& m) {
					return m.requester;
				 }
			       )
		     { start(); }
};

void Dowser::start() {
	bus.subscribe<Msg::Init
		     >([this](Msg::Init const& init) {
		rpc = &init.rpc;
		self_id = init.self_id;
		return Ev::lift();
	});
	bus.subscribe<Msg::RequestDowser
		     >([this](Msg::RequestDowser const& r) {
		auto run = std::make_shared<Run>( bus, r.requester
						, r.fromid, r.toid
						);
		return Ev::lift().then([this]() {
			return wait_for_rpc(rpc);
		}).then([this, run]() {
			return Boss::concurrent(run->run(*rpc, self_id));
		});
	});

	cmdimpl = Util::make_unique<CommandImpl>(bus);
}

Dowser::~Dowser() =default;
Dowser::Dowser(S::Bus& bus_
	      ) : bus(bus_)
		, rpc(nullptr)
		, self_id()
		{ start(); }

}}
