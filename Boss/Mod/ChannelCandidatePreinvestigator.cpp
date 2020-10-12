#include"Boss/Mod/ChannelCandidatePreinvestigator.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/PreinvestigateChannelCandidates.hpp"
#include"Boss/Msg/ProposeChannelCandidates.hpp"
#include"Boss/Msg/RequestConnect.hpp"
#include"Boss/Msg/RequestDowser.hpp"
#include"Boss/Msg/ResponseConnect.hpp"
#include"Boss/Msg/ResponseDowser.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ln/Amount.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"
#include<functional>
#include<map>
#include<queue>

namespace {

auto const min_amount = Ln::Amount::btc(0.005);

}

namespace Boss { namespace Mod {

class ChannelCandidatePreinvestigator::Impl {
private:
	S::Bus& bus;
	ModG::ReqResp<Msg::RequestDowser, Msg::ResponseDowser> dowser;

	void start() {
		using std::placeholders::_1;
		bus.subscribe<Msg::PreinvestigateChannelCandidates
			     >(std::bind(&Impl::on_preinv, this, _1));
		bus.subscribe<Msg::ResponseConnect
			     >(std::bind(&Impl::on_connect, this, _1));
	}

	/* A sequence of candidates that are being preinvestigated.  */
	class Case : public std::enable_shared_from_this<Case> {
	private:
		S::Bus& bus;
		Impl& impl;
		std::queue<Msg::ProposeChannelCandidates> candidates;
		std::size_t remaining;

		Case( S::Bus& bus_
		    , Impl& impl_
		    , Msg::PreinvestigateChannelCandidates const& p
		    ) : bus(bus_)
		      , impl(impl_)
		      {
			remaining = p.max_candidates;
			for (auto const& c : p.candidates)
				candidates.emplace(c);
		}

	public:
		static
		std::shared_ptr<Case>
		create( S::Bus& bus
		      , Impl& impl
		      , Msg::PreinvestigateChannelCandidates const& p
		      ) {
			/* Private constructor, cannot use
			 * std::make_shared.  */
			return std::shared_ptr<Case>(
				new Case(bus, impl, p)
			);
		}

		static
		Ev::Io<void> run(std::shared_ptr<Case> self) {
			return self->loop().then([self]() {
				return Ev::lift();
			});
		}
	private:
		Ev::Io<void> loop() {
			/* If no more, end of preinvestigation.  */
			if (candidates.empty())
				return Ev::lift();
			if (remaining == 0)
				return Ev::lift();

			auto& curr = candidates.front();
			auto& node = curr.proposal;
			auto node_s = std::string(node);

			/* Add to cases being tracked.  */
			impl.add_case(node_s, shared_from_this());

			/* Trigger connect.  */
			return bus.raise(Msg::RequestConnect{
				std::move(node_s)
			});
		}
	public:
		Ev::Io<void> on_connect_response(bool success) {
			auto curr = std::move(candidates.front());
			candidates.pop();
			if (success) {
				--remaining;
				return on_success(curr);
			} else
				return loop();
		}
	private:
		Ev::Io<void>
		on_success_connect(Msg::ProposeChannelCandidates p) {
			auto self = shared_from_this();
			auto curr = std::make_shared<
				Msg::ProposeChannelCandidates
			>(std::move(p));
			return Ev::lift().then([self, curr]() {
				auto msg = Msg::RequestDowser{
					nullptr,
					curr->proposal,
					curr->patron
				};
				return self->impl.dowser.execute(std::move(
					msg
				));
			}).then([self, curr](Msg::ResponseDowser r) {
				if (r.amount >= min_amount)
					return self->on_success(*curr);

				return Boss::log( self->bus, Debug
						, "ChannelCandidate"
						  "Preinvestigator: "
						  "Rejecting %s (patron %s) "
						  "due to low flow %s "
						  "(minimum %s)."
						, std::string(curr->proposal)
							.c_str()
						, std::string(curr->patron)
							.c_str()
						, std::string(r.amount)
							.c_str()
						, std::string(min_amount)
							.c_str()
						).then([self]() {
					return self->loop();
				});
			});
		}
		Ev::Io<void>
		on_success(Msg::ProposeChannelCandidates const& p) {
			auto self = shared_from_this();
			return Boss::log( bus, Info
					, "ChannelCandidatesPreinvestigator: "
					  "Proposing %s (patron %s)"
					, std::string(p.proposal).c_str()
					, std::string(p.patron).c_str()
					).then([self, p]() {
				return self->bus.raise(p);
			}).then([self]() {
				return self->loop();
			});
		}
	};

	std::map<std::string, std::shared_ptr<Case>> cases;
	void add_case( std::string const& node
		     , std::shared_ptr<Case> c
		     ) {
		cases[node] = std::move(c);
	}

	Ev::Io<void> on_preinv(Msg::PreinvestigateChannelCandidates const& p) {
		auto c = Case::create(bus, *this, p);
		return Case::run(c);
	}
	Ev::Io<void> on_connect(Msg::ResponseConnect const& r) {
		/* Look it up.  */
		auto it = cases.find(r.node);
		if (it == cases.end())
			return Ev::lift();

		/* Remove it from cases.  */
		auto c = std::move(it->second);
		cases.erase(it);

		/* Execute it.  */
		return c->on_connect_response(r.success).then([c]() {
			return Ev::lift();
		});
	}

public:
	Impl() =delete;
	Impl(S::Bus& bus_
	    ) : bus(bus_)
	      , dowser( bus_
		      , [](Msg::RequestDowser& r, void* p) { r.requester = p; }
		      , [](Msg::ResponseDowser& r) { return r.requester; }
		      )
	      { start(); }
};


ChannelCandidatePreinvestigator::ChannelCandidatePreinvestigator(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus))
	{ }
ChannelCandidatePreinvestigator::ChannelCandidatePreinvestigator
		(ChannelCandidatePreinvestigator&& o)
	: pimpl(std::move(o.pimpl))
	{ }

ChannelCandidatePreinvestigator::~ChannelCandidatePreinvestigator() { }

}}
