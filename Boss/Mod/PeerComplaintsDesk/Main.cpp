#include"Boss/Mod/PeerComplaintsDesk/Exempter.hpp"
#include"Boss/Mod/PeerComplaintsDesk/Main.hpp"
#include"Boss/Mod/PeerComplaintsDesk/Recorder.hpp"
#include"Boss/Mod/PeerComplaintsDesk/Unmanager.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/ChannelDestruction.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/OnchainFee.hpp"
#include"Boss/Msg/ProvideStatus.hpp"
#include"Boss/Msg/RaisePeerComplaint.hpp"
#include"Boss/Msg/RequestPeerMetrics.hpp"
#include"Boss/Msg/ResponsePeerMetrics.hpp"
#include"Boss/Msg/SolicitPeerComplaints.hpp"
#include"Boss/Msg/SolicitStatus.hpp"
#include"Boss/Msg/Timer10Minutes.hpp"
#include"Boss/Msg/TimerTwiceDaily.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Util/make_unique.hpp"
#include"Util/stringify.hpp"

namespace {

/* Maximum age, in seconds, to retain complaints.  */
auto constexpr max_complaint_age = double(14 * 24 * 60 * 60);
/* Maximum age, in seconds, to retain complaints for closed channels.  */
auto constexpr max_closed_complaint_age = double(90 * 24 * 60 * 60);

/* Number of non-ignored complaints before we decide to close.  */
auto constexpr max_acceptable_complaints = std::size_t(3);

/* Number of seconds we will request for unilateral closing timeout.  */
auto constexpr channel_close_timeout = std::size_t(3 * 60);
/* How strongly do we insist on our own feerate.  */
auto const fee_negotiation_step = std::string("1");

}

namespace Boss { namespace Mod { namespace PeerComplaintsDesk {

class Main::Impl {
private:
	Unmanager unmanager;
	Exempter exempter;

	S::Bus& bus;
	Sqlite3::Db db;

	Boss::Mod::Rpc* rpc;

	bool soliciting;
	bool should_close;
	bool fees_low;

	std::map<Ln::NodeId, std::string> exempted_nodes;
	std::map<Ln::NodeId, std::string> pending_complaints;

	std::unique_ptr<Msg::SolicitPeerComplaints> solicit_message;

	ModG::ReqResp< Msg::RequestPeerMetrics
		     , Msg::ResponsePeerMetrics
		     > metrics_rr;

	void start() {
		rpc = nullptr;
		soliciting = false;
		should_close = false;
		fees_low = false;
		exempted_nodes.clear();
		pending_complaints.clear();

		bus.subscribe<Msg::DbResource
			     >([this](Msg::DbResource const& m) {
			db = m.db;
			return db.transact().then([](Sqlite3::Tx tx) {
				Recorder::initialize(tx);
				tx.commit();

				return Ev::lift();
			});
		});
		bus.subscribe<Msg::Init
			     >([this](Msg::Init const& m) {
			rpc = &m.rpc;
			return Ev::lift();
		});

		bus.subscribe<Msg::OnchainFee
			     >([this](Msg::OnchainFee const& m) {
			fees_low = m.fees_low;
			return Ev::lift();
		});

		bus.subscribe<Msg::TimerTwiceDaily
			     >([this](Msg::TimerTwiceDaily const& _) {
			return Boss::concurrent(solicit())
			     + Boss::concurrent(cleanup())
			     ;
		});
		bus.subscribe<Msg::Timer10Minutes
			     >([this](Msg::Timer10Minutes const& _) {
			if (!should_close || !rpc)
				return Ev::lift();
			should_close = false;
			return Boss::concurrent(check_close());
		});

		bus.subscribe<Msg::RaisePeerComplaint
			     >([this](Msg::RaisePeerComplaint const& m) {
			if (!soliciting)
				return Ev::lift();

			auto it = pending_complaints.find(m.peer);
			if (it == pending_complaints.end())
				pending_complaints[m.peer] = m.complaint;
			else
				/* Multiple complaints?
				 * Merge into single complaint.
				 */
				it->second += std::string("; ") + m.complaint;
			return Ev::lift();
		});

		bus.subscribe<Msg::ChannelDestruction
			     >([this](Msg::ChannelDestruction const& m) {
			return Boss::concurrent(on_channel_destroy(m.peer));
		});

		bus.subscribe<Msg::SolicitStatus
			     >([this](Msg::SolicitStatus const& _) {
			return provide_status();
		});
	}

	Ev::Io<void> solicit() {
		if (soliciting)
			return Ev::lift();

		soliciting = true;
		pending_complaints.clear();

		return Ev::lift().then([this]() {

			return exempter.get_exemptions();
		}).then([this](std::map<Ln::NodeId, std::string> new_exempted_nodes) {
			exempted_nodes = std::move(new_exempted_nodes);

			return metrics_rr.execute(Msg::RequestPeerMetrics{});
		}).then([this](Msg::ResponsePeerMetrics metrics) {
			solicit_message = Util::make_unique<Msg::SolicitPeerComplaints>();
			solicit_message->day3 = metrics.day3;
			solicit_message->weeks2 = metrics.weeks2;
			solicit_message->month1 = metrics.month1;

			return Boss::log( bus, Debug
					, "PeerComplaintsDesk: Soliciting complaints."
					);
		}).then([this]() {
			auto tmp_solicit_message = std::move(solicit_message);
			solicit_message = nullptr;
			return bus.raise(std::move(*tmp_solicit_message));
		}).then([this]() {
			return db.transact();
		}).then([this](Sqlite3::Tx tx) {
			auto act = Ev::lift();

			for (auto const& c : pending_complaints) {
				auto const& peer = c.first;
				auto const& complaint = c.second;

				auto ex_it = exempted_nodes.find(peer);
				if (ex_it == exempted_nodes.end()) {
					/* Peer is not exempted, record complaint.  */
					Recorder::add_complaint( tx
							       , peer, complaint
							       );
					act += Boss::log( bus, Debug
							, "PeerComplaintsDesk: "
							  "%s: %s"
							, Util::stringify(peer).c_str()
							, complaint.c_str()
							);
				} else {
					/* Peer is exempted, record complaint as ignored.  */
					Recorder::add_ignored_complaint( tx
								       , peer, complaint
								       , ex_it->second
								       );
					act += Boss::log( bus, Debug
							, "PeerComplaintsDesk: "
							  "%s: %s "
							  "[IGNORED: %s]"
							, Util::stringify(peer).c_str()
							, complaint.c_str()
							, ex_it->second.c_str()
							);
				}
			}
			tx.commit();

			/* Finished soliciting.  */
			soliciting = false;
			/* Schedule closures for next 10-minute timer.  */
			should_close = true;
			/* Do not waste memory.  */
			exempted_nodes.clear();
			pending_complaints.clear();

			/* Return logs.  */
			return act;
		});
	}
	Ev::Io<void> cleanup() {
		return db.transact().then([](Sqlite3::Tx tx) {
			Recorder::cleanup(tx, max_complaint_age, max_closed_complaint_age);
			tx.commit();

			return Ev::lift();
		});
	}
	Ev::Io<void> check_close() {
		return Ev::lift().then([this]() {
			if (!fees_low)
				return Boss::log( bus, Info
						, "PeerComplaintsDesk: "
						  "Fees are not known to be low, "
						  "will not close high-complaints channels."
						);
			return actual_check_close();
		});
	}
	Ev::Io<void> actual_check_close() {
		return db.transact().then([this](Sqlite3::Tx tx) {
			auto complaints = Recorder::check_complaints(tx);
			tx.commit();

			auto const& unmanaged = unmanager.get_unmanaged();

			auto act = Ev::lift();
			act += Boss::log( bus, Debug
					, "PeerComplaintsDesk: "
					  "Checking for channels to close."
					);

			auto msg = std::string();
			auto first = true;
			auto to_close = std::vector<Ln::NodeId>();
			for (auto& c : complaints) {
				if (first)
					first = false;
				else
					msg += ", ";
				msg += Util::stringify(c.first)
				     + " ("
				     + Util::stringify(c.second)
				     ;
				if (unmanaged.count(c.first) != 0) {
					msg += "; !UNMANAGED!)";
					continue;
				}
				msg +=
				     + ")"
				     ;
				if (c.second >= max_acceptable_complaints)
					to_close.push_back(c.first);
			}
			if (!first)
				act += Boss::log( bus, Debug
						, "PeerComplaintsDesk: Complaints: %s"
						, msg.c_str()
						);

			msg = std::string();
			first = true;
			for (auto& p : to_close) {
				if (first)
					first = false;
				else
					msg += ", ";
				msg += Util::stringify(p);
				act += Boss::concurrent(close_channel(p));
			}
			if (!first)
				act += Boss::log( bus, Info
						, "PeerComplaintsDesk: Closing: %s"
						, msg.c_str()
						);
			return act;
		});
	}
	Ev::Io<void> close_channel(Ln::NodeId const& p) {
		return Ev::lift().then([this, p]() {
			auto parms = Json::Out()
				.start_object()
					.field("id", Util::stringify(p))
					.field("unilateraltimeout", channel_close_timeout)
					.field("fee_negotiation_step", fee_negotiation_step)
				.end_object()
				;
			return rpc->command("close", parms);
		}).then([](Jsmn::Object _) {
			return Ev::lift();
		}).catching<RpcError>([this, p](RpcError e) {
			return Boss::log( bus, Error
					, "PeerComplaintsDesk: close %s error: %s"
					, Util::stringify(p).c_str()
					, Util::stringify(e.error).c_str()
					);
		});
	}
	Ev::Io<void> on_channel_destroy(Ln::NodeId const& p) {
		return db.transact().then([p](Sqlite3::Tx tx) {
			Recorder::channel_closed(tx, p);
			tx.commit();
			return Ev::lift();
		});
	}

	Ev::Io<void> provide_status() {
		return db.transact().then([this](Sqlite3::Tx tx) {
			auto complaints = Recorder::get_all_complaints(tx);
			auto closed_complaints = Recorder::get_closed_complaints(tx);
			tx.commit();

			auto peer_complaints_j = jsonify(complaints);
			auto closed_peer_complaints_j = jsonify(closed_complaints);

			return bus.raise(Msg::ProvideStatus{
					"peer_complaints",
					std::move(peer_complaints_j)
			       })
			     + bus.raise(Msg::ProvideStatus{
					"closed_peer_complaints",
					std::move(closed_peer_complaints_j)
			       })
			     ;
		});
	}
	Json::Out jsonify(std::map< Ln::NodeId
				  , std::vector<std::string>
				  > const& map) {
		auto rv = Json::Out();
		auto obj = rv.start_object();
		for (auto const& ps : map) {
			auto arr = obj.start_array(Util::stringify(ps.first));
			for (auto const& s : ps.second)
				arr.entry(s);
			arr.end_array();
		}
		obj.end_object();
		return rv;
	}

public:
	Impl() =delete;
	Impl(Impl const&) =delete;
	Impl(Impl&&) =delete;

	explicit
	Impl(S::Bus& bus_) : unmanager(bus_)
			   , exempter(bus_, unmanager)
			   , bus(bus_)
			   , metrics_rr( bus_
				       , [](Msg::RequestPeerMetrics& m, void* p) {
						m.requester = p;
					 }
				       , [](Msg::ResponsePeerMetrics& m) {
						return m.requester;
					 }
				       )
			   { start(); }
};

Main::Main(Main&&) =default;
Main::~Main() =default;

Main::Main(S::Bus& bus) : pimpl(Util::make_unique<Impl>(bus)) { }

}}}
