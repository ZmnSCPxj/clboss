#ifndef BOSS_MOD_PEERMETRICIAN_HPP
#define BOSS_MOD_PEERMETRICIAN_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::PeerMetrician
 *
 * @brief This is not a poet.
 * Instead, this queries the peer statistician for raw
 * data, then generates metrics we might find useful
 * for judging if we should close channels to the peer
 * or not.
 */
class PeerMetrician {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	PeerMetrician() =delete;

	PeerMetrician(PeerMetrician&&);
	~PeerMetrician();

	explicit
	PeerMetrician(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_PEERMETRICIAN_HPP) */
