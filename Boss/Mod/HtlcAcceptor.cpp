#include"Boss/Mod/HtlcAcceptor.hpp"
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/ManifestHook.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/ProvideHtlcAcceptedDeferrer.hpp"
#include"Boss/Msg/ReleaseHtlcAccepted.hpp"
#include"Boss/Msg/SolicitHtlcAcceptedDeferrer.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ev/map.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include"Util/Str.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"
#include<algorithm>
#include<inttypes.h>
#include<set>
#include<vector>

namespace {

/* Number of seconds to defer response of htlc accepted.  */
auto const defer_time = double(6.0);

}

namespace Boss { namespace Mod {

class HtlcAcceptor::Impl {
private:
	S::Bus& bus;
	Boss::Mod::Waiter& waiter;

	/* Deferrer functions.  */
	std::vector<std::function<Ev::Io<bool>(
		Ln::HtlcAccepted::Request const&
	)>> deferrers;
	bool soliciting;
	bool solicited;

	/* `htlc_accepted` hooks that we are informing to
	 * deferrers.
	 */
	std::set<std::uint64_t> deferring;
	/* `htlc_accepted` hooks that we know are deferred.
	 */
	std::set<std::uint64_t> deferred;

	void start() {
		bus.subscribe<Msg::CommandRequest
			     >([this](Msg::CommandRequest const& req) {
			if (req.command != "htlc_accepted")
				return Ev::lift();
			return parse_payload(req.id, req.params
			).then([this
			      ](std::shared_ptr<Ln::HtlcAccepted::Request
					       > ptr) {
				if (!ptr)
					return Ev::lift();
				return htlc_accepted(std::move(ptr));
			});
		});
		bus.subscribe<Msg::ProvideHtlcAcceptedDeferrer
			     >([this
			       ](Msg::ProvideHtlcAcceptedDeferrer const& m) {
			if (!soliciting)
				return Ev::lift();
			deferrers.push_back(m.deferrer);
			return Ev::lift();
		});
		bus.subscribe<Msg::ReleaseHtlcAccepted
			     >([this] (Msg::ReleaseHtlcAccepted const& r) {
			return release_htlc_accepted(r.response);
		});

		bus.subscribe<Msg::Manifestation
			     >([this](Msg::Manifestation const& _) {
			return bus.raise(Msg::ManifestHook{"htlc_accepted"});
		});
	}

	Ev::Io<void> solicit() {
		return Ev::yield().then([this]() {
			if (solicited)
				return Ev::lift();
			if (soliciting)
				/* Busy-wait.  */
				return solicit();
			soliciting = true;
			return bus.raise(Msg::SolicitHtlcAcceptedDeferrer{ }
					).then([this]() {
				soliciting = false;
				solicited = true;
				return Ev::lift();
			});
		});
	}

	Ev::Io<std::shared_ptr<Ln::HtlcAccepted::Request>>
	parse_payload( std::uint64_t id
		     , Jsmn::Object const& payload
		     ) {
		auto rv = std::make_shared<Ln::HtlcAccepted::Request>();
		rv->id = id;
		try {
			auto onion = payload["onion"];
			auto htlc = payload["htlc"];

			rv->incoming_payload = Util::Str::hexread(std::string(
				onion["payload"]
			));
			rv->incoming_amount = Ln::Amount(std::string(
				htlc["amount"]
			));
			rv->incoming_cltv = std::uint32_t(double(
				htlc["cltv_expiry"]
			));

			if (onion.has("type"))
				rv->type_tlv = "tlv" == std::string(
					onion["type"]
				);
			else
				rv->type_tlv = false;

			if (onion.has("short_channel_id")) {
				rv->next_channel = Ln::Scid(std::string(
					onion["short_channel_id"]
				));
				rv->next_amount = Ln::Amount(std::string(
					onion["forward_amount"]
				));
				rv->next_cltv = std::uint32_t(double(
					onion["outgoing_cltv_value"]
				));
			} else {
				rv->next_channel = Ln::Scid(nullptr);
				rv->next_amount = rv->incoming_amount;
				rv->next_cltv = rv->incoming_cltv;
			}

			rv->next_onion = Util::Str::hexread(std::string(
				onion["next_onion"]
			));

			rv->payment_hash = Sha256::Hash(std::string(
				htlc["payment_hash"]
			));
			if (onion.has("payment_secret"))
				rv->payment_secret = Ln::Preimage(std::string(
					onion["payment_secret"]
				));
		} catch (Jsmn::TypeError const& e) {
			return Boss::log( bus, Error
					, "HtlcAcceptor: Unexpected payload "
					  "of htlc_accepted: %s"
					, Util::stringify(payload).c_str()
					).then([this, e]() {
				throw e;
				return Ev::lift(std::shared_ptr<
					Ln::HtlcAccepted::Request
				>());
			});
		}
		return Ev::lift(std::move(rv));
	}
	Ev::Io<void>
	htlc_accepted(std::shared_ptr<Ln::HtlcAccepted::Request> req) {
		auto id = req->id;
		return solicit().then([this, req]() {
			return Boss::log( bus, Debug
					, "HtlcAcceptor: "
					  "HTLC %" PRIu64 " (%s) arrived."
					, req->id
					, std::string(req->payment_hash)
						.c_str()
					);
		}).then([this, req]() {
			deferring.insert(req->id);
			auto f = [ this
				 , req
				 ](std::function<Ev::Io<bool>(Ln::HtlcAccepted::Request const&)> deferrer) {
				return deferrer(*req);
			};
			return Ev::map(std::move(f), deferrers);
		}).then([this, id](std::vector<bool> results) {
			auto it = deferring.find(id);
			/* There is a race condition where one of the
			 * deferrers takes too long to decide, then one
			 * of the *other* deferrers has already given
			 * the result.
			 * So if it is no longer in the deferring set,
			 * quit.
			 */
			if (it == deferring.end())
				return Ev::lift();
			deferring.erase(it);
			deferred.insert(id);

			/* If anyone wanted to defer, wait then raise.  */
			if (std::any_of( results.begin(), results.end()
				       , [](bool x) { return x; }
				       )) {
				auto act = waiter.wait(defer_time)
					 + finish(id)
					 ;
				return Boss::concurrent(act);
			}

			return finish(id);
		});
	}
	Ev::Io<void> finish(std::uint64_t id) {
		return Ev::lift().then([this, id]() {
			auto it = deferred.find(id);
			if (it == deferred.end())
				/* Somebody got to it first,
				 * silently finish.  */
				return Ev::lift();
			return bus.raise(Msg::ReleaseHtlcAccepted{
				Ln::HtlcAccepted::Response::cont(id)
			});
		});
	}

	Ev::Io<void>
	release_htlc_accepted(Ln::HtlcAccepted::Response const& resp) {
		/* First get the id and check if it is in either
		 * deferred or deferring.  */
		auto id = resp.id();
		auto found = false;
		auto it1 = deferred.find(id);
		if (it1 != deferred.end()) {
			deferred.erase(it1);
			found = true;
		} else {
			auto it2 = deferring.find(id);
			if (it2 != deferring.end()) {
				deferring.erase(it2);
				found = true;
			}
		}

		/* Silently fail.  */
		if (!found)
			return Ev::lift();

		/* Generate hook result.  */
		auto os = std::ostringstream();
		auto result = Json::Out();
		if (resp.is_cont()) {
			os << "Continue.";
			result = Json::Out()
				.start_object()
					.field("result", "continue")
				.end_object()
				;
		} else if (resp.is_fail()) {
			auto const& msg = resp.fail_message();
			auto msg_s = Util::Str::hexdump(&msg[0], msg.size());
			os << "Fail: " << msg_s;
			result = Json::Out()
				.start_object()
					.field("result", "fail")
					.field("failure_message", msg_s)
				.end_object()
				;
		} else {
			auto key = std::string(resp.resolve_preimage());
			os << "Resolve: " << key;
			result = Json::Out()
				.start_object()
					.field("result", "resolve")
					.field("payment_key", key)
				.end_object()
				;
		}

		return Boss::log( bus, Debug
				, "HtlcAcceptor: HTLC %" PRIu64 ": %s"
				, id
				, os.str().c_str()
				)
		     + bus.raise(Msg::CommandResponse{id, std::move(result)})
		     ;
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	explicit
	Impl( S::Bus& bus_
	    , Boss::Mod::Waiter& waiter_
	    ) : bus(bus_), waiter(waiter_)
	      , soliciting(false)
	      , solicited(false)
	      { start(); }
};

HtlcAcceptor::HtlcAcceptor(HtlcAcceptor&&) =default;
HtlcAcceptor::~HtlcAcceptor() =default;

HtlcAcceptor::HtlcAcceptor( S::Bus& bus
			  , Boss::Mod::Waiter& waiter
			  ) : pimpl(Util::make_unique<Impl>(bus, waiter))
			    { }

}}
