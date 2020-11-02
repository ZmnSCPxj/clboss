#include"Boss/Mod/ChannelCandidateMatchmaker.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/PatronizeChannelCandidate.hpp"
#include"Boss/Msg/PreinvestigateChannelCandidates.hpp"
#include"Boss/Msg/ProposeChannelCandidates.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include<memory>
#include<sstream>

namespace Boss { namespace Mod {

/* This is effectively a single run of the matchmaker.  */
class ChannelCandidateMatchmaker::Run
		: public std::enable_shared_from_this<Run> {
private:
	S::Bus& bus;
	Boss::Mod::Rpc& rpc;

	Ln::NodeId proposal;
	std::queue<Ln::NodeId> guide;

	explicit
	Run( S::Bus& bus_
	   , Boss::Mod::Rpc& rpc_
	   , Ln::NodeId proposal_
	   , std::queue<Ln::NodeId> guide_
	   ) : bus(bus_)
	     , rpc(rpc_)
	     , proposal(std::move(proposal_))
	     , guide(std::move(guide_))
	     { }

public:
	Run() =delete;
	Run(Run&&) =delete;
	Run(Run const&) =delete;

	static
	std::shared_ptr<Run>
	create( S::Bus& bus
	      , Boss::Mod::Rpc& rpc
	      , Ln::NodeId proposal
	      , std::queue<Ln::NodeId> guide
	      ) {
		return std::shared_ptr<Run>(
			new Run( bus
			       , rpc
			       , std::move(proposal)
			       , std::move(guide)
			       )
		);
	}

	Ev::Io<void> run() {
		auto self = shared_from_this();
		return self->core_run().then([self]() {
			return Ev::lift();
		});
	}
private:
	Ev::Io<void> core_run() {
		return Ev::yield().then([this]() {
			if (guide.empty())
				/* Failed.  */
				return Boss::log( bus, Debug
						, "ChannelCandidateMatchmaker:"
						  " Could not find patron for "
						  "%s."
						, std::string(proposal
							     ).c_str()
						);

			return step();
		});
	}
	Ev::Io<void> step() {
		auto target = std::move(guide.front());
		guide.pop();

		auto parms = Json::Out()
			.start_object()
				.field("id", std::string(target))
				.field("fromid", std::string(proposal))
				.field("msatoshi", "1msat")
				/* No real idea how to think about
				 * riskfactor.
				 */
				.field("riskfactor", 10)
				.field("fuzzpercent", 0)
			.end_object()
			;
		return rpc.command("getroute", std::move(parms)
				  ).then([this](Jsmn::Object res) {
			auto patron = Ln::NodeId();
			try {
				patron = Ln::NodeId(std::string(
					res["route"][0]["id"]
				));
			} catch (Jsmn::TypeError const&) {
				auto os = std::ostringstream();
				os << res;
				return Boss::log( bus, Error
						, "ChannelCandidateMatchmaker:"
						  " Unexpected result from "
						  "getroute: %s"
						, os.str().c_str()
						).then([]()
							-> Ev::Io<void>{
					throw RpcError( "getroute"
						      , Jsmn::Object()
						      );
				});
			}
			auto act = Ev::lift();
			act += Boss::log( bus, Debug
					, "ChannelCandidateMatchmaker: "
					  "Matched proposal %s to patron %s."
					, std::string(proposal).c_str()
					, std::string(patron).c_str()
					);
			auto propose = Msg::ProposeChannelCandidates{
				std::move(proposal), std::move(patron)
			};
			auto preinv = Msg::PreinvestigateChannelCandidates{
				{std::move(propose)},
				1
			};
			act += bus.raise(std::move(preinv));
			return act;
		}).catching<RpcError>([this](RpcError const&) {
			/* Try next.  */
			return core_run();
		});
	}
};

void ChannelCandidateMatchmaker::start() {
	bus.subscribe<Msg::Init
		     >([this](Msg::Init const& init) {
		rpc = &init.rpc;
		return Ev::lift();
	});
	bus.subscribe<Msg::PatronizeChannelCandidate
		     >([this](Msg::PatronizeChannelCandidate const& m) {
		if (!rpc)
			return Ev::lift();

		/* Construct queue.  */
		auto q = std::queue<Ln::NodeId>();
		for (auto const& n : m.guide)
			q.push(n);
		/* Create run object.  */
		auto run = Run::create(bus, *rpc, m.proposal, std::move(q));
		return Boss::concurrent(run->run());
	});
}

}}
