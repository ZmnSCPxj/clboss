#ifndef BOSS_MOD_RECONNECTOR_HPP
#define BOSS_MOD_RECONNECTOR_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::Reconnector
 *
 * @brief Checks if all peers are disconnected, and if so,
 * triggers soliciting more connections.
 *
 * @desc Some nodes on the network will disconnect you after
 * a while if you do not do anything more than download
 * gossip from them.
 * As gossip download is necessary for us to have at least
 * some "ground truth" on what nodes are good to connect to,
 * we should attempt to reconnect if peers disconnect.
 *
 * This is particularly important since DNS seeding is
 * fairly unreliable and we might take several minutes to
 * find a node via DNS seed.
 */
class Reconnector {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Reconnector() =delete;
	Reconnector(Reconnector const&) =delete;

	explicit
	Reconnector(S::Bus&);
	Reconnector(Reconnector&&);
	~Reconnector();
};

}}

#endif /* !defined(BOSS_MOD_RECONNECTOR_HPP) */
