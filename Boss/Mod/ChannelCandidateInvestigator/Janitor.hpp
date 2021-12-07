#ifndef BOSS_MODD_CHANNELCANDIDATEINVESTIGATR_JANITOR_HPP
#define BOSS_MODD_CHANNELCANDIDATEINVESTIGATR_JANITOR_HPP
#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/RequestDowser.hpp"
#include"Boss/Msg/ResponseDowser.hpp"
#include<memory>

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator { class Secretary; }}}
namespace Ev { template<typename a> class Io; }
namespace Ln { class Amount; }
namespace Ln { class NodeId; }
namespace S { class Bus; }
namespace Sqlite3 { class Tx; }

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator {

/** class Boss::Mod::ChannelCandidateInvestigator::Janitor
 *
 * @brief Cleans up the candidates list of the secretary, removing
 * those that have too little dowsed capacity to their patrons.
 * Channel creator will reject such candidates anyway, might as well
 * clean them up outright.
 */
class Janitor {
private:
	S::Bus& bus;
	ModG::ReqResp<Msg::RequestDowser, Msg::ResponseDowser> dowser;

public:
	Janitor() =delete;
	Janitor(Janitor const&) =delete;
	Janitor(Janitor&&) =delete;

	explicit
	Janitor(S::Bus& bus);

	/* Clean up channels below the given min_channel size, and log
	 * report.
	 */
	Ev::Io<void> clean_up( Secretary& secretary
			     , std::shared_ptr<Sqlite3::Tx> tx
			     , Ln::Amount min_channel
			     );
	/* Do a quick single OK/NG check.  True = OK, False = reject.  */
	Ev::Io<bool> check_acceptable( Ln::NodeId proposal
				     , Ln::NodeId patron
				     , Ln::Amount min_channel
				     );
};

}}}

#endif /* BOSS_MODD_CHANNELCANDIDATEINVESTIGATR_JANITOR_HPP */
