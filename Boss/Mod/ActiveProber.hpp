#ifndef BOSS_MOD_ACTIVEPROBER_HPP
#define BOSS_MOD_ACTIVEPROBER_HPP

#include"Ln/NodeId.hpp"
#include"Secp256k1/Random.hpp"

namespace Boss { namespace Mod { namespace ChannelCandidateInvestigator {
	class Main;
}}}
namespace Boss { namespace Mod { class Rpc; }}
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ActiveProber
 *
 * @brief Constructs outgoing payments that are doomed to fail
 * (they are to random hashes that with high probability have
 * preimages not known by anyone at all).
 *
 * @desc The prober does not directly report to anyone, but
 * the `SendpayResultsMonitor` will see its outgoing attempt,
 * and add it to statistics stored by `PeerStatistician`.
 *
 * The prober probes peers in reaction to
 * `Boss::Msg::ProbeActively`.
 * It creates a route starting with the channel at that peer,
 * then goes to *some* destination.
 */
class ActiveProber {
private:
	S::Bus& bus;
	ChannelCandidateInvestigator::Main& investigator;
	Rpc* rpc;

	Ln::NodeId self_id;

	Secp256k1::Random random;

	class Run;

	void start();

public:
	ActiveProber() =delete;
	ActiveProber(ActiveProber&&) =delete;
	ActiveProber(ActiveProber const&) =delete;

	explicit
	ActiveProber( S::Bus& bus_
		    , ChannelCandidateInvestigator::Main& investigator_
		    ) : bus(bus_)
		      , investigator(investigator_)
		      , rpc(nullptr)
		      , self_id()
		      , random()
		      { start(); }
};

}}

#endif /* !defined(BOSS_MOD_ACTIVEPROBER_HPP) */
