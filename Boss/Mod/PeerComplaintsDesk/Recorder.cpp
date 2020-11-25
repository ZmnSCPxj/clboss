#include"Boss/Mod/PeerComplaintsDesk/Recorder.hpp"
#include"Ev/now.hpp"
#include"Ln/NodeId.hpp"
#include"Sqlite3.hpp"
#include<sstream>

#include<iostream>

namespace Boss { namespace Mod { namespace PeerComplaintsDesk {

void Recorder::initialize(Sqlite3::Tx& tx) {
	tx.query_execute(R"QRY(
	CREATE TABLE IF NOT EXISTS
	       "PeerComplaintsDesk_peers"
	     ( peerdbid INTEGER PRIMARY KEY
	     , nodeid TEXT NOT NULL
	     );
	CREATE UNIQUE INDEX IF NOT EXISTS
	       "PeerComplaintsDesk_peers_id"
	    ON "PeerComplaintsDesk_peers"(nodeid)
	     ;

	CREATE TABLE IF NOT EXISTS
	       "PeerComplaintsDesk_complaints"
	     ( complaintid INTEGER PRIMARY KEY
	     , peerdbid INTEGER NOT NULL
	     , time REAL NOT NULL
	     , complaint TEXT NOT NULL
	     , ignored INTEGER NOT NULL
	     , ignoredreason TEXT NULL
	     , FOREIGN KEY(peerdbid)
	       REFERENCES "PeerComplaintsDesk_peers"(peerdbid)
	       ON DELETE CASCADE
	     );
	CREATE INDEX IF NOT EXISTS
	       "PeerComplaintsDesk_complaints_time"
	    ON "PeerComplaintsDesk_complaints"(time)
	     ;
	)QRY");
}
void Recorder::cleanup(Sqlite3::Tx& tx, double age) {
	tx.query(R"QRY(
	DELETE FROM "PeerComplaintsDesk_complaints"
	 WHERE time < :mintime
	     ;
	)QRY")
		.bind(":mintime", Ev::now() - age)
		.execute()
		;

	/* Filter out peerdbids that have no more complaints.
	 *
	 * There is probably a nice single query for this,
	 * but I cannot remember how.
	 */
	auto peerdbids = std::vector<std::uint64_t>();
	auto fetch = tx.query(R"QRY(
	SELECT peerdbid FROM "PeerComplaintsDesk_peers";
	)QRY").execute();
	for (auto& r : fetch)
		peerdbids.push_back(r.get<std::uint64_t>(0));

	for (auto peerdbid : peerdbids) {
		auto count = std::size_t();
		auto fetch2 = tx.query(R"QRY(
		SELECT COUNT(*) FROM "PeerComplaintsDesk_complaints"
		 WHERE peerdbid = :peerdbid
		     ;
		)QRY")
			.bind(":peerdbid", peerdbid)
			.execute()
			;
		for (auto& r : fetch2)
			count = r.get<std::size_t>(0);
		if (count != 0)
			continue;

		tx.query(R"QRY(
		DELETE FROM "PeerComplaintsDesk_peers"
		 WHERE peerdbid = :peerdbid
		     ;
		)QRY")
			.bind(":peerdbid", peerdbid)
			.execute()
			;
	}
}

namespace {

std::uint64_t get_peerdbid(Sqlite3::Tx& tx, Ln::NodeId const& nid) {
	auto nid_s = std::string(nid);
	auto fetch = tx.query(R"QRY(
	SELECT peerdbid FROM "PeerComplaintsDesk_peers"
	 WHERE nodeid = :nodeid
	     ;
	)QRY")
		.bind(":nodeid", nid_s)
		.execute()
		;
	for (auto& r : fetch)
		return r.get<std::uint64_t>(0);

	tx.query(R"QRY(
	INSERT INTO "PeerComplaintsDesk_peers"
	      (nodeid)
	VALUES(:nodeid);
	)QRY")
		.bind(":nodeid", nid_s)
		.execute()
		;

	auto peerdbid = std::uint64_t();
	auto fetch2 = tx.query(R"QRY(
	SELECT peerdbid FROM "PeerComplaintsDesk_peers"
	 WHERE nodeid = :nodeid
	     ;
	)QRY")
		.bind(":nodeid", nid_s)
		.execute()
		;
	for (auto& r : fetch2)
		peerdbid = r.get<std::uint64_t>(0);

	return peerdbid;
}

}

void Recorder::add_complaint( Sqlite3::Tx& tx
			    , Ln::NodeId const& peer
			    , std::string const& complaint
			    ) {
	auto peerdbid = get_peerdbid(tx, peer);
	tx.query(R"QRY(
	INSERT INTO "PeerComplaintsDesk_complaints"
	      (  peerdbid,  time,  complaint, ignored, ignoredreason)
	VALUES( :peerdbid, :time, :complaint, 0      , NULL);
	)QRY")
		.bind(":peerdbid", peerdbid)
		.bind(":time", Ev::now())
		.bind(":complaint", complaint)
		.execute()
		;
}

void Recorder::add_ignored_complaint( Sqlite3::Tx& tx
				    , Ln::NodeId const& peer
				    , std::string const& complaint
				    , std::string const& ignoredreason
				    ) {
	auto peerdbid = get_peerdbid(tx, peer);
	tx.query(R"QRY(
	INSERT INTO "PeerComplaintsDesk_complaints"
	      (  peerdbid,  time,  complaint, ignored,  ignoredreason)
	VALUES( :peerdbid, :time, :complaint, 1      , :ignoredreason);
	)QRY")
		.bind(":peerdbid", peerdbid)
		.bind(":time", Ev::now())
		.bind(":complaint", complaint)
		.bind(":ignoredreason", ignoredreason)
		.execute()
		;
}

std::map< Ln::NodeId
	, std::vector<std::string>
	> Recorder::get_all_complaints(Sqlite3::Tx& tx) {
	auto rv = std::map<Ln::NodeId, std::vector<std::string>>();

	auto fetch = tx.query(R"QRY(
	SELECT nodeid, time, complaint, ignored, ignoredreason
	  FROM "PeerComplaintsDesk_peers" NATURAL JOIN
	       "PeerComplaintsDesk_complaints";
	)QRY").execute();
	for (auto& r : fetch) {
		auto peer = Ln::NodeId(r.get<std::string>(0));
		auto time = r.get<double>(1);
		auto complaint = r.get<std::string>(2);
		auto ignored = r.get<int>(3) == 1 ? true : false;
		auto ignoredreason = std::string();
		if (ignored)
			ignoredreason = r.get<std::string>(4);

		auto os = std::ostringstream();
		/* FIXME: Human-readable time.  */
		os << time << ": " << complaint;
		if (ignored)
			os << " [IGNORED: " << ignoredreason << "]";

		rv[peer].push_back(os.str());
	}

	return rv;
}
std::map< Ln::NodeId
	, std::size_t
	> Recorder::check_complaints(Sqlite3::Tx& tx) {
	auto rv = std::map<Ln::NodeId, std::size_t>();

	auto fetch = tx.query(R"QRY(
	SELECT nodeid
	  FROM "PeerComplaintsDesk_peers" NATURAL JOIN
	       "PeerComplaintsDesk_complaints"
	 WHERE ignored = 0;
	)QRY").execute();
	for (auto& r : fetch) {
		auto peer = Ln::NodeId(r.get<std::string>(0));
		auto it = rv.find(peer);
		if (it == rv.end())
			rv[peer] = 1;
		else
			++it->second;
	}

	return rv;
}

}}}
