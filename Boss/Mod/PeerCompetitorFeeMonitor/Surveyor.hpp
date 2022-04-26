#ifndef BOSS_MOD_PEERCOMPETITORFEEMONITOR_SURVEYOR_HPP
#define BOSS_MOD_PEERCOMPETITORFEEMONITOR_SURVEYOR_HPP

#include"Jsmn/Object.hpp"
#include"Ln/Amount.hpp"
#include"Ln/NodeId.hpp"
#include"Ln/Scid.hpp"
#include"Stats/WeightedMedian.hpp"
#include<cstdint>
#include<memory>
#include<string>
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
	bool have_listchannels_destination;

	std::size_t samples;
	Stats::WeightedMedian<std::uint32_t, Ln::Amount> bases;
	Stats::WeightedMedian<std::uint32_t, Ln::Amount> proportionals;

	Jsmn::Object channels;
	Jsmn::Object::iterator it;

	/* Progress reporting.  */
	double prev_time;
	std::size_t count;
	std::size_t total_count;

	Surveyor( S::Bus& bus_
		, Boss::Mod::Rpc& rpc_
		, Ln::NodeId self_id_
		, Ln::NodeId peer_id_
		, bool have_listchannels_destination_
		) : bus(bus_)
		  , rpc(rpc_)
		  , self_id(self_id_)
		  , peer_id(peer_id_)
		  , have_listchannels_destination(have_listchannels_destination_)
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
	      /* Does the node have a `listchannels` command
	       * with `destination` parameter?  */
	      , bool have_listchannels_destination
	      ) {
		return std::shared_ptr<Surveyor>(
			new Surveyor( bus
				    , rpc
				    , std::move(self_id)
				    , std::move(peer_id)
				    , have_listchannels_destination
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
	std::string one_channel(Jsmn::Object const&);
	Ev::Io<std::unique_ptr<Result>> extract();

	/* Back-compatibility mode.  */
	Ev::Io<void> old_loop();
};

}}}

#endif /* !defined(BOSS_MOD_PEERCOMPETITORFEEMONITOR_SURVEYOR_HPP) */
