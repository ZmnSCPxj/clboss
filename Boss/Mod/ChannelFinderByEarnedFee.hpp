#ifndef BOSS_MOD_CHANNELFINDERBYEARNEDFEE_HPP
#define BOSS_MOD_CHANNELFINDERBYEARNEDFEE_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ChannelFinderByEarnedFee
 *
 * @brief Searches for channel candidates by looking up peers
 * which have been lucrative for our outgoing fee earnings.
 *
 * @desc We select a peer with high outgoing fee earnings, and
 * select a peer of that peer.
 * The logic is that a peer with high outgoing fee earnings is
 * a popular node as destination, and we want to have greater
 * capacity in its "general direction".
 */
class ChannelFinderByEarnedFee {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	ChannelFinderByEarnedFee() =delete;
	ChannelFinderByEarnedFee(ChannelFinderByEarnedFee const&) =delete;

	ChannelFinderByEarnedFee(ChannelFinderByEarnedFee&&);
	~ChannelFinderByEarnedFee();
	explicit
	ChannelFinderByEarnedFee(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_CHANNELFINDERBYEARNEDFEE_HPP) */
