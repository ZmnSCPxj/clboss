#ifndef BOSS_MOD_PEERCOMPETITORFEEMONITOR_MAIN_HPP
#define BOSS_MOD_PEERCOMPETITORFEEMONITOR_MAIN_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod { namespace PeerCompetitorFeeMonitor {

/** class Boss::Mod::PeerCompetitorFeeMonitor::Main
 *
 * @brief periodically checks the fees that go into
 * each of our peers.
 */
class Main {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Main() =delete;

	Main(Main&&);
	Main& operator=(Main&&);
	~Main();

	explicit
	Main(S::Bus& bus);
};

}}}

#endif /* !defined(BOSS_MOD_PEERCOMPETITORFEEMONITOR_MAIN_HPP) */
