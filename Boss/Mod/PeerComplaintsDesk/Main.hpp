#ifndef BOSS_MOD_PEERCOMPLAINTSDESK_MAIN_HPP
#define BOSS_MOD_PEERCOMPLAINTSDESK_MAIN_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod { namespace PeerComplaintsDesk {

/** class Boss::Mod::PeerComplaintsDesk::Main
 *
 * @brief main object for the `PeerComplaintsDesk`, which holds
 * all the other objects.
 */
class Main {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Main() =delete;
	Main(Main const&) =delete;

	Main(Main&&);
	~Main();

	explicit
	Main(S::Bus& bus);
};

}}}

#endif /* !defined(BOSS_MOD_PEERCOMPLAINTSDESK_MAIN_HPP) */
