#include"Boss/Mod/UnmanagedManager.hpp"
#include"Boss/Msg/CommandFail.hpp"
#include"Boss/Msg/CommandRequest.hpp"
#include"Boss/Msg/CommandResponse.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/Manifestation.hpp"
#include"Boss/Msg/ManifestCommand.hpp"
#include"Boss/Msg/ProvideUnmanagement.hpp"
#include"Boss/Msg/ProvideStatus.hpp"
#include"Boss/Msg/SolicitStatus.hpp"
#include"Boss/Msg/SolicitUnmanagement.hpp"
#include"Boss/concurrent.hpp"
#include"Ev/Io.hpp"
#include"Ev/foreach.hpp"
#include"Ln/NodeId.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Util/make_unique.hpp"
#include<map>
#include<set>
#include<sstream>
#include<stdexcept>
#include<vector>

namespace {

/* Splits a comma-separated string of tags into multiple tags.  */
std::set<std::string> split_tags(std::string const& tagspec) {
	auto rv = std::set<std::string>();

	auto is = std::istringstream(tagspec);
	while (is) {
		auto str = std::string();
		std::getline(is, str, ',');
		if (str != "")
			rv.insert(str);
	}

	return rv;
}
/* Combines multiple tags into a single tagspec.  */
std::string combine_tags(std::set<std::string> const& tags) {
	auto os = std::ostringstream();
	auto first = true;
	for (auto const& tag : tags) {
		if (first)
			first = false;
		else
			os << ", ";
		os << tag;
	}
	return os.str();
}

}

namespace Boss { namespace Mod {

class UnmanagedManager::Impl {
private:
	S::Bus& bus;

	Sqlite3::Db db;

	bool soliciting;
	std::map< std::string, std::function<Ev::Io<void>(Ln::NodeId const&, bool)>
		> tag_informs;

	void start() {
		soliciting = false;
		tag_informs.clear();

		/* Initialization and solicitation.  */
		bus.subscribe<Msg::DbResource
			     >([this](Msg::DbResource const& m) {
			db = m.db;
			return initialize() + solicit();
		});
		bus.subscribe<Msg::ProvideUnmanagement
			     >([this](Msg::ProvideUnmanagement const& m) {
			if (!soliciting)
				return Ev::lift();

			tag_informs[m.tag] = m.inform;
			return initial_inform(m.tag);
		});

		/* Command handling.  */
		bus.subscribe<Msg::Manifestation
			     >([this](Msg::Manifestation const& m) {
			return bus.raise(Msg::ManifestCommand{
				"clboss-unmanage",
				"nodeid tags",
				"Specify that a {nodeid} will not be fully managed by "
				"CLBOSS, disabling the management of {tags}.",
				false
			});
		});
		bus.subscribe<Msg::CommandRequest
			     >([this](Msg::CommandRequest const& m) {
			if (m.command != "clboss-unmanage")
				return Ev::lift();
			return unmanage(m.id, m.params);
		});

		/* Status report.  */
		bus.subscribe<Msg::SolicitStatus
			     >([this](Msg::SolicitStatus const& _) {
			return db.transact().then([this](Sqlite3::Tx tx) {
				auto out = Json::Out();
				auto obj = out.start_object();
				obj.field( "comment"
					 , "Use 'clboss-unmanage' to disable management of "
					   "some nodes."
					 );

				auto nodes = std::vector<Ln::NodeId>();
				auto fetch = tx.query(R"QRY(
				SELECT nodeid
				  FROM "UnmanagedManager_nodes"
				     ;
				)QRY")
					.execute()
					;
				for (auto& r : fetch)
					nodes.push_back(Ln::NodeId(r.get<std::string>(0)));

				for (auto const& n : nodes) {
					auto tags = get_tags(tx, n);
					/* Do not display nodes that are fully managed.  */
					if (tags.empty())
						continue;
					obj.field( std::string(n)
						 , combine_tags(tags)
						 );
				}

				obj.end_object();

				tx.commit();
				return bus.raise(Msg::ProvideStatus{
					"unmanaged", std::move(out)
				});
			});
		});
	}

	Ev::Io<void> initialize() {
		return db.transact().then([](Sqlite3::Tx tx) {
			tx.query_execute(R"QRY(
			CREATE TABLE IF NOT EXISTS
			       "UnmanagedManager_nodes"
			     ( nodedbid INTEGER PRIMARY KEY NOT NULL
			     , nodeid TEXT UNIQUE NOT NULL
			     );
			CREATE TABLE IF NOT EXISTS
			       "UnmanagedManager_tags"
			     ( nodedbid INTEGER NOT NULL
			     , tag TEXT NOT NULL
			     , FOREIGN KEY(nodedbid)
			       REFERENCES "UnmanagedManager_nodes"
			       (nodedbid)
			       ON DELETE CASCADE
			     );
			CREATE UNIQUE INDEX IF NOT EXISTS
			       "UnmanagedManager_tagsidx"
			    ON "UnmanagedManager_tags"
			       (nodedbid, tag);
			CREATE INDEX IF NOT EXISTS
			       "UnmanagedManager_tags_tagidx"
			    ON "UnmanagedManager_tags"
			       (tag);
			)QRY");

			tx.commit();
			return Ev::lift();
		});
	}
	Ev::Io<void> solicit() {
		return Ev::lift().then([this]() {
			soliciting = true;
			return bus.raise(Msg::SolicitUnmanagement{});
		}).then([this]() {
			soliciting = false;
			return Ev::lift();
		});
	}

	/* Call the inform function for this tag during initial solicitation.  */
	Ev::Io<void> initial_inform(std::string const& tag) {
		return db.transact().then([this, tag](Sqlite3::Tx tx) {
			auto nodes = std::vector<Ln::NodeId>();
			auto fetch = tx.query(R"QRY(
			SELECT nodeid
			  FROM "UnmanagedManager_nodes" NATURAL JOIN "UnmanagedManager_tags"
			 WHERE tag = :tag
			     ;
			)QRY")
				.bind(":tag", tag)
				.execute()
				;
			for (auto& r : fetch)
				nodes.push_back(Ln::NodeId(r.get<std::string>(0)));
			tx.commit();

			auto inform = tag_informs[tag];
			auto f = [inform](Ln::NodeId const& node) {
				return inform(node, true);
			};
			return Ev::foreach(std::move(f), std::move(nodes));
		});
	}

	Ev::Io<void> unmanage(Ln::CommandId id, Jsmn::Object const& params) {
		return db.transact().then([this, params](Sqlite3::Tx tx) {
			auto nodeid = Ln::NodeId();
			auto new_tags = std::set<std::string>();
			parse_params(nodeid, new_tags, params);

			/* Get tags.  */
			auto old_tags = get_tags(tx, nodeid);

			auto act = Ev::lift();
			/* For every old tag that is not in new tag, remove.  */
			for (auto const& tag : old_tags) {
				auto it = new_tags.find(tag);
				if (it != new_tags.end())
					continue;
				act += Boss::concurrent(tag_informs[tag](nodeid, false));
			}
			/* For every new tag that is not in old tag, add.  */
			for (auto const& tag : new_tags) {
				auto it = old_tags.find(tag);
				if (it != old_tags.end())
					continue;
				act += Boss::concurrent(tag_informs[tag](nodeid, true));
			}
			/* Update tags.  */
			set_tags(tx, nodeid, new_tags);

			tx.commit();

			return act;
		}).then([this, id]() {
			return bus.raise(Msg::CommandResponse{
				id,
				Json::Out::empty_object()
			});
		}).catching<std::exception>([this, id](std::exception const& e) {
			return bus.raise(Msg::CommandFail{
				id, -32602,
				std::string("Parameter error: ") + e.what(),
				Json::Out::empty_object()
			});
		});
	}
	void parse_params( Ln::NodeId& nodeid
			 , std::set<std::string>& tags
			 , Jsmn::Object const& params
			 ) {
		auto nodeid_j = Jsmn::Object();
		auto tags_j = Jsmn::Object();
		if (params.is_array()) {
			nodeid_j = params[0];
			tags_j = params[1];
		} else {
			nodeid_j = params["nodeid"];
			tags_j = params["tags"];
		}
		nodeid = Ln::NodeId(std::string(nodeid_j));
		tags = split_tags(std::string(tags_j));
		for (auto const& tag : tags) {
			auto it = tag_informs.find(tag);
			if (it == tag_informs.end())
				throw std::runtime_error(
					std::string("Unknown tag: ") + tag
				);
		}
	}

	/* Database access.  */
	std::uint64_t get_nodedbid( Sqlite3::Tx& tx
				  , Ln::NodeId const& nodeid
				  ) {
		auto fetch = tx.query(R"QRY(
		SELECT nodedbid
		  FROM "UnmanagedManager_nodes"
		 WHERE nodeid = :nodeid
		     ;
		)QRY")
			.bind(":nodeid", std::string(nodeid))
			.execute()
			;
		for (auto& r : fetch)
			return r.get<std::uint64_t>(0);

		/* Not found.  */
		tx.query(R"QRY(
		INSERT OR IGNORE
		  INTO "UnmanagedManager_nodes"
		 VALUES( NULL
		       , :nodeid
		       );
		)QRY")
			.bind(":nodeid", std::string(nodeid))
			.execute()
			;

		/* Redo.  */
		return get_nodedbid(tx, nodeid);
	}
	std::set<std::string> get_tags( Sqlite3::Tx& tx
				      , Ln::NodeId const& nodeid
				      ) {
		auto nodedbid = get_nodedbid(tx, nodeid);
		auto fetch = tx.query(R"QRY(
		SELECT tag
		  FROM "UnmanagedManager_tags"
		 WHERE nodedbid = :nodedbid
		     ;
		)QRY")
			.bind(":nodedbid", nodedbid)
			.execute()
			;
		auto rv = std::set<std::string>();
		for (auto& r : fetch)
			rv.insert(r.get<std::string>(0));
		return rv;
	}
	void set_tags( Sqlite3::Tx& tx
		     , Ln::NodeId const& nodeid
		     , std::set<std::string> const& tags
		     ) {
		auto nodedbid = get_nodedbid(tx, nodeid);
		/* First, erase.  */
		tx.query(R"QRY(
		DELETE FROM "UnmanagedManager_tags"
		 WHERE nodedbid = :nodedbid
		     ;
		)QRY")
			.bind(":nodedbid", nodedbid)
			.execute()
			;
		/* Then insert.  */
		for (auto const& tag : tags)
			tx.query(R"QRY(
			INSERT INTO "UnmanagedManager_tags"
			VALUES( :nodedbid
			      , :tag
			      );
			)QRY")
				.bind(":nodedbid", nodedbid)
				.bind(":tag", tag)
				.execute()
				;
	}

public:
	Impl() =delete;
	Impl(Impl&&) =delete;

	explicit
	Impl(S::Bus& bus_) : bus(bus_) { start(); }
};

UnmanagedManager::UnmanagedManager(UnmanagedManager&&) =default;
UnmanagedManager::~UnmanagedManager() =default;

UnmanagedManager::UnmanagedManager(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
