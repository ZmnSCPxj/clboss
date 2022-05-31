#include"Boss/Mod/MoveFundsCommand.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/CommandFail.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/ManifestCommand.hpp"
#include"Boss/Msg/RequestMoveFunds.hpp"
#include"Boss/Msg/ResponseMoveFunds.hpp"
#include"Boss/concurrent.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include"Util/make_unique.hpp"

namespace Boss { namespace Mod {

class MoveFundsCommand::Impl {
private:
	ModG::ReqResp< Msg::RequestMoveFunds
		     , Msg::ResponseMoveFunds
		     > movefunds;

	void start(S::Bus& bus) {
		bus.subscribe<Msg::Manifestation
			     >([&bus](Msg::Manifestation const& _) {
			return bus.raise(Msg::ManifestCommand{
				"clboss-movefunds",
				"amount sourcenode destnode feebudget",
				"Move {amount} from a channel with "
				"{sourcenode} to a channel with {destnode}, "
				"with no more than {feebudget}.  "
				"{amount} and {feebudget} must have 'msat' "
				"suffixes, and {sourcenode} and {destnode} "
				"must be node IDs.  "
				"THIS COMMAND IS FOR DEBUGGING CLBOSS AND "
				"SHOULD NOT BE USED FOR MANUAL MANAGEMENT.",
				false
			});
		});
		bus.subscribe<Msg::CommandRequest
			     >([this, &bus](Msg::CommandRequest const& r) {
			if (r.command != "clboss-movefunds")
				return Ev::lift();
			auto parms = std::make_shared<Jsmn::Object>(
				r.params
			);
			auto id = r.id;
			return Boss::concurrent(do_movefunds(
				bus, std::move(parms), id
			));
		});
	}

	Ev::Io<void>
	do_movefunds( S::Bus& bus
		    , std::shared_ptr<Jsmn::Object> pparms
		    , std::uint64_t id
		    ) {
		return Ev::lift().then([this, &bus, pparms, id]() {
			auto msg = Msg::RequestMoveFunds();
			try {
				auto amt = Jsmn::Object();
				auto src = Jsmn::Object();
				auto dst = Jsmn::Object();
				auto fee = Jsmn::Object();
				auto& parms = *pparms;
				if (parms.is_array() && parms.size() == 4) {
					amt = parms[0];
					src = parms[1];
					dst = parms[2];
					fee = parms[3];
				} else {
					amt = parms["amount"];
					src = parms["sourcenode"];
					dst = parms["destnode"];
					fee = parms["feebudget"];
				}
				msg.amount = Ln::Amount(std::string(amt));
				msg.source = Ln::NodeId(std::string(src));
				msg.destination = Ln::NodeId(std::string(dst));
				msg.fee_budget = Ln::Amount(std::string(fee));
			} catch (std::exception const& _) {
				return bus.raise(Msg::CommandFail{
					id, -32602, "Invalid parameters.",
					Json::Out()
						.start_object()
							.field( "params"
							      , *pparms
							      )
						.end_object()
				});
			}
			return movefunds.execute( std::move(msg)
			).then([&bus, id](Msg::ResponseMoveFunds rsp) {
				auto result = Json::Out()
				.start_object()
					.field( "amount_moved"
					      , std::string(rsp.amount_moved)
					      )
					.field( "fee_spent"
					      , std::string(rsp.fee_spent)
					      )
				.end_object()
				;
				return bus.raise(Msg::CommandResponse{
					id, std::move(result)
				});
			});
		});
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	explicit
	Impl( S::Bus& bus
	    ) : movefunds(bus)
	      { start(bus); }
};

MoveFundsCommand::MoveFundsCommand(MoveFundsCommand&&) =default;
MoveFundsCommand::~MoveFundsCommand() =default;

MoveFundsCommand::MoveFundsCommand(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
