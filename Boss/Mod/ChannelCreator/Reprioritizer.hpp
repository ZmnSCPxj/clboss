#ifndef BOSS_MOD_CHANNELCREATOR_REPRIORITIZER_HPP
#define BOSS_MOD_CHANNELCREATOR_REPRIORITIZER_HPP

#include<functional>
#include<memory>
#include<utility>
#include<vector>

namespace Ev { template<typename a> class Io; }
namespace Ln { class NodeId; }
namespace Net { class IPAddrOrOnion; }
namespace Net { class IPBinnerIF; }
namespace Secp256k1 { class SignerIF; }

namespace Boss { namespace Mod { namespace ChannelCreator {

/** class Boss::Mod::ChannelCreator::Reprioritizer
 *
 * @brief Object to rearrange a list of proposal-patron
 * pairs based on IP binning.
 *
 * @desc More specifically, proposals that have the same
 * bin as existing channels or higher-priority proposals
 * have their priorities reduced.
 *
 * Proposals are prioritized according to the order in
 * which they are in the given input vector.
 * Earlier entries are considered higher-priority.
 */
class Reprioritizer {
private:
	class Run;

	Secp256k1::SignerIF& signer;
	std::unique_ptr<Net::IPBinnerIF> binner;
	std::function< Ev::Io<std::unique_ptr<Net::IPAddrOrOnion>>(Ln::NodeId)
		     > get_node_addr;
	std::function< Ev::Io<std::vector<Ln::NodeId>>()> get_peers;

public:
	Reprioritizer() =delete;
	Reprioritizer(Reprioritizer const&) =delete;
	Reprioritizer(Reprioritizer&&) =delete;

	~Reprioritizer();

	explicit
	Reprioritizer( Secp256k1::SignerIF& signer_
		     , std::unique_ptr<Net::IPBinnerIF> binner_
		     , std::function< Ev::Io<std::unique_ptr<Net::IPAddrOrOnion>>(Ln::NodeId)
				    > get_node_addr_
		     , std::function< Ev::Io<std::vector<Ln::NodeId>>()> get_peers_
		     );

	/** Boss::Mod::ChannelCreator::Reprioritizer::reprioritize
	 *
	 * @brief Rearranges the given list of proposal-patron pairs
	 * based on IP binning.
	 */
	Ev::Io<std::vector<std::pair<Ln::NodeId, Ln::NodeId>>>
	reprioritize(std::vector<std::pair<Ln::NodeId, Ln::NodeId>> proposals);
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCREATOR_REPRIORITIZER_HPP) */
