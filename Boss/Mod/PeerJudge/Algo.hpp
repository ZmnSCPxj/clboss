#ifndef BOSS_MOD_PEERJUDGE_ALGO_HPP
#define BOSS_MOD_PEERJUDGE_ALGO_HPP

#include"Boss/Mod/PeerJudge/Info.hpp"
#include<cstdint>
#include<memory>
#include<map>
#include<string>
#include<vector>

namespace Json { class Out; }

namespace Boss { namespace Mod { namespace PeerJudge {

/** class Boss::Mod::PeerJudge::Algo
 *
 * @brief Executes the peer-judgment algorithm.
 *
 * @desc The `judge_peers` member function is the main algorithm.
 * The `get_status` member function provides the status of the
 * latest iteration of the algorithm.
 * Finally, the `get_close` function provides the IDs of the
 * peers we judge should be closed.
 */
class Algo {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Algo();
	~Algo();
	Algo(Algo const&) =delete;
	Algo(Algo&&) =delete;

	Json::Out get_status() const;
	std::map<Ln::NodeId, std::string> get_close() const;
	void judge_peers( std::vector<Info> infos
			, double feerate
			, std::uint64_t reopen_weight
			);
};

}}}

#endif /* !defined(BOSS_MOD_PEERJUDGE_ALGO_HPP) */
