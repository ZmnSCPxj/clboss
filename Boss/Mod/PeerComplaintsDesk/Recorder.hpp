#ifndef BOSS_MOD_PEERCOMPLAINTSDESK_RECORDER_HPP
#define BOSS_MOD_PEERCOMPLAINTSDESK_RECORDER_HPP

#include<map>
#include<cstddef>
#include<string>
#include<vector>

namespace Ln { class NodeId; }
namespace Sqlite3 { class Tx; }

namespace Boss { namespace Mod { namespace PeerComplaintsDesk {

/** namespace Boss::Mod::PeerComplaintsDesk::Recorder
 *
 * @brief Module handles database transactions to record and
 * keep track of complaints about our peers.
 */
namespace Recorder {
	/** Boss::Mod::PeerComplaintsDesk::Recorder::initialize
	 *
	 * @brief ensures the database has the needed tables and
	 * indices.
	 */
	void initialize(Sqlite3::Tx&);
	/** Boss::Mod::PeerComplaintsDesk::Recorder::cleanup
	 *
	 * @brief Cleans up old complaints older than the specified
	 * age.
	 */
	void cleanup(Sqlite3::Tx&, double age);
	/** Boss::Mod::PeerComplaintsDesk::Recorder::add_complaint
	 *
	 * @brief adds a complaint.
	 */
	void add_complaint( Sqlite3::Tx&
			  , Ln::NodeId const&
			  , std::string const& complaint
			  );
	/** Boss::Mod::PeerComplaintsDesk::Recorder::add_ignored_complaint
	 *
	 * @brief adds a complaint, but marked as ignored.
	 * It will be returned in `get_all_complaints`, but will not be
	 * counted in `check_complaints`.
	 */
	void add_ignored_complaint( Sqlite3::Tx&
				  , Ln::NodeId const&
				  , std::string const& complaint
				  , std::string const& ignoredreason
				  );

	/** Boss::Mod::PeerComplaintsDesk::Recorder::get_all_complaints
	 *
	 * @brief gathers all remembered complaints for all peers,
	 * including ignored ones.
	 * Adds useful human annotations: time, whether ignored (and
	 * why ignored).
	 */
	std::map< Ln::NodeId
		, std::vector<std::string>
		> get_all_complaints(Sqlite3::Tx&);
	/** Boss::Mod::PeerComplaintsDesk::Recorder::check_complaints
	 *
	 * @brief counts non-ignored complaints and provides the number
	 * of non-ignored complaints for each peer.
	 */
	std::map< Ln::NodeId
		, std::size_t
		> check_complaints(Sqlite3::Tx&);
};

}}}

#endif /* !defined(BOSS_MOD_PEERCOMPLAINTSDESK_RECORDER_HPP) */
