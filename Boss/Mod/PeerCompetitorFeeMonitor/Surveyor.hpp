#ifndef BOSS_MOD_PEERCOMPETITORFEEMONITOR_SURVEYOR_HPP
#define BOSS_MOD_PEERCOMPETITORFEEMONITOR_SURVEYOR_HPP

#include"Stats/WeightedMedian.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include"Ln/Scid.hpp"
#include<cstdint>
#include<memory>
#include<queue>
#include<utility>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace PeerCompetitorFeeMonitor {

/** class Boss::Mod::PeerCompetitorFeeMonitor::Surveyor
 *
 * @brief temporary object for determining the
 * median feerates going into the specified peer.
 */
class Surveyor : public std::enable_shared_from_this<Surveyor> {
private:
	S::Bus& bus;
	Boss::Mod::Rpc& rpc;
	Ln::NodeId self_id;
	Ln::NodeId peer_id;

	std::queue<Ln::Scid> to_process;

	std::size_t samples;
	Stats::WeightedMedian<std::uint32_t, Ln::Amount> bases;
	Stats::WeightedMedian<std::uint32_t, Ln::Amount> proportionals;

	Surveyor( S::Bus& bus_
		, Boss::Mod::Rpc& rpc_
		, Ln::NodeId self_id_
		, Ln::NodeId peer_id_
		) : bus(bus_)
		  , rpc(rpc_)
		  , self_id(self_id_)
		  , peer_id(peer_id_)
		  , samples(0)
		  { }

public:
	struct Result {
		Ln::NodeId peer_id;
		std::uint32_t median_base;
		std::uint32_t median_proportional;
	};

	Surveyor() =delete;
	Surveyor(Surveyor&&) =delete;

	static
	std::shared_ptr<Surveyor>
	create( S::Bus& bus
	      , Boss::Mod::Rpc& rpc
	      /* Our own node ID.  */
	      , Ln::NodeId self_id
	      /* The node to survey.  */
	      , Ln::NodeId peer_id
	      ) {
		return std::shared_ptr<Surveyor>(
			new Surveyor( bus
				    , rpc
				    , std::move(self_id)
				    , std::move(peer_id)
				    )
		);
	}

	/* Returns nullptr if there were no samples
	 * for that peer.
	 */
	Ev::Io<std::unique_ptr<Result>>
	run();

private:
	Ev::Io<void> core_run();
	Ev::Io<void> loop();
	Ev::Io<std::unique_ptr<Result>> extract();
};

}}}

#endif /* !defined(BOSS_MOD_PEERCOMPETITORFEEMONITOR_SURVEYOR_HPP) */
