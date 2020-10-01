#ifndef BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_SECRETARY_HPP
#define BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_SECRETARY_HPP

#include<cstdint>
#include<cstdlib>
#include<string>
#include<utility>
#include<vector>

namespace Boss { namespace Msg { class ProposeChannelCandidates; }}
namespace Ln { class NodeId; }
namespace Sqlite3 { class Tx; }

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator {

/** class Boss::Mod::ChannelCandidateInvestigator::Secretary
 *
 * @brief object responsible for handling the database
 * tables.
 *
 * @desc Handles queries from and updates to the database.
 * Note that the Manager object is the one that owns the
 * database transaction, and ultimately commits or rolls it
 * back.
 */
class Secretary {
public:
	/** Secretary::initialize
	 *
	 * @brief initializes the database if it is not yet
	 * initialized.
	 * No-op if the database is already initialized.
	 */
	void initialize(Sqlite3::Tx& tx);

	/** Secretary::add_candidate
	 *
	 * @brief adds the candidate to the set of candidates to
	 * investigate.
	 * If it is already under investigation, this is a no-op.
	 */
	void add_candidate( Sqlite3::Tx& tx
			  , Msg::ProposeChannelCandidates const& candidate
			  );

	/** Secretary::get_nonnegative_candidates_count
	 *
	 * @brief counts the number of candidates in the table
	 * whose score is not negative.
	 */
	std::size_t get_nonnegative_candidates_count(Sqlite3::Tx& tx);

	/** Secretary::get_for_investigation
	 *
	 * @brief Selects a random number of candidates to
	 * investigate, up to the given `max_candidates`.
	 */
	std::vector<Ln::NodeId>
	get_for_investigation( Sqlite3::Tx& tx
			     , std::size_t max_candidates
			     );

	/** Secretary::update_score
	 *
	 * @brief Increases or decreases the score of the candidate.
	 * If the score ends up below the given `minimum_score`, delete
	 * the candidate.
	 * If the score ends up above the given `maximum_score`,
	 * saturate the score to `maximum_score`.
	 * If the candidate does not exist, this is a no-op.
	 */
	void update_score( Sqlite3::Tx& tx
			 , Ln::NodeId const& node
			 , std::int64_t delta_score
			 , std::int64_t minimum_score
			 , std::int64_t maximum_score
			 );

	/** Secretary::get_for_channeling
	 *
	 * @brief Selects all positive-score candidates and sorts them
	 * by score.
	 * If there are no positive-score candidates (all are 0 or
	 * negative), selects them all and sorts by score.
	 */
	std::vector<Msg::ProposeChannelCandidates>
	get_for_channeling(Sqlite3::Tx& tx);

	/** Secretary::remove_candidate
	 *
	 * @brief removes a candidate.
	 */
	void remove_candidate(Sqlite3::Tx& tx, Ln::NodeId const& node);

	/** Secretary::report
	 *
	 * @brief create a human-readable report.
	 */
	std::string report(Sqlite3::Tx& tx);

	/** Secretary::get_all
	 *
	 * @brief get all nodes under investigation and
	 * their current scores, in order from highest
	 * to lowest.
	 */
	std::vector<std::pair<Ln::NodeId, std::int64_t>>
	get_all(Sqlite3::Tx& tx);

	/** Secretary::is_candidate
	 *
	 * @brief determine if the given node is already a
	 * candidate under investigation.
	 */
	bool is_candidate(Sqlite3::Tx& tx, Ln::NodeId const& node);
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCANDIDATEINVESTIGATOR_SECRETARY_HPP) */
