#ifndef BOSS_MOD_PEERJUDGE_DATAGATHERER_HPP
#define BOSS_MOD_PEERJUDGE_DATAGATHERER_HPP

#include<functional>
#include<memory>
#include<vector>

namespace Boss { namespace Mod { namespace PeerJudge { struct Info; }}}
namespace Ev { template<typename a> class Io; }
namespace Ln { class NodeId; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace PeerJudge {

/** class Boss::Mod::PeerJudge::DataGatherer
 *
 * @brief Gathers information about all peers
 * using the bus.
 *
 * @desc
 * This looks at the data from the most recent
 * given `timeframe_seconds`; peers whose channels
 * are shorter-lived than this are ignored.
 * The function `get_min_age` is called and should
 * return a minimum possible age for the channel
 * with a node.
 * Every time it gathers data, it calls the
 * given `fun` with all peers and with the latest
 * available feerate.
 */
class DataGatherer {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	DataGatherer( S::Bus& bus
		    , double timeframe_seconds
		    , std::function<Ev::Io<double>(Ln::NodeId)> get_min_age
		    , std::function<Ev::Io<void>(std::vector<Info>, double)> fun
		    );
	~DataGatherer();
	DataGatherer(DataGatherer const&) =delete;
	DataGatherer(DataGatherer&&) =delete;
};

}}}

#endif /* !defined(BOSS_MOD_PEERJUDGE_DATAGATHERER_HPP) */
