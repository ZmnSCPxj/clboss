#include"Boss/Mod/ChannelCandidateInvestigator/Gumshoe.hpp"
#include"Boss/Msg/RequestConnect.hpp"
#include"Boss/Msg/ResponseConnect.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<set>

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator {

class Gumshoe::Impl {
private:
	S::Bus& bus;

	std::function< Ev::Io<void>( Ln::NodeId
				   , bool
				   )
		     > report;

	void start() {
		bus.subscribe<Msg::ResponseConnect>([this](Msg::ResponseConnect const& rc) {
			return on_response_connect(rc);
		});
	}

	/* Nodes we are investigating.  */
	std::set<Ln::NodeId> cases;

	Ev::Io<void> investigate_core(Ln::NodeId n) {
		cases.insert(n);
		return Boss::log( bus, Debug
				, "ChannelCandidateInvestigator: "
				  "Investigating %s."
				, std::string(n).c_str()
				).then([this, n]() {
			return bus.raise(Msg::RequestConnect{std::string(n)});
		});
	}

	Ev::Io<void> on_response_connect(Msg::ResponseConnect const& rc) {
		/* Was it a plain nodeid like what we generate?  */
		if (!Ln::NodeId::valid_string(rc.node))
			return Ev::lift();

		/* Is it one of the cases we are watching?  */
		auto node = Ln::NodeId(rc.node);
		auto it = cases.find(node);
		if (it == cases.end())
			return Ev::lift();

		auto success = rc.success;

		/* Done with this case.  */
		cases.erase(it);
		return Boss::log( bus, Debug
				, "ChannelCandidateInvestigator: %s is %s."
				, rc.node.c_str()
				, success ? "online" : "offline"
				).then([this, node, success]() {
			return report(node, success);
		});
	}

public:
	Impl( S::Bus& bus_
	    ) : bus(bus_)
	      , report(nullptr)
	      {
		start();
	}

	void set_report_func(std::function< Ev::Io<void>( Ln::NodeId
							, bool
							)
					  > report_) {
		report = std::move(report_);
	}

	Ev::Io<void> investigate(Ln::NodeId n) {
		return Boss::concurrent(Ev::lift().then([this, n]() {
			return investigate_core(n);
		}));
	}
};

Gumshoe::Gumshoe(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }
Gumshoe::~Gumshoe() { }


void Gumshoe::set_report_func(std::function< Ev::Io<void>( Ln::NodeId
							 , bool
							 )
					   > report) {
	return pimpl->set_report_func(std::move(report));
}
Ev::Io<void> Gumshoe::investigate(Ln::NodeId n) {
	return pimpl->investigate(std::move(n));
}

}}}
