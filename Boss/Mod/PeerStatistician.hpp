#ifndef BOSS_MOD_PEERSTATISTICIAN_HPP
#define BOSS_MOD_PEERSTATISTICIAN_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

class PeerStatistician {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	PeerStatistician() =delete;
	PeerStatistician(PeerStatistician const&) =delete;

	PeerStatistician(PeerStatistician&&);
	~PeerStatistician();

	explicit
	PeerStatistician(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_PEERSTATISTICIAN_HPP) */
