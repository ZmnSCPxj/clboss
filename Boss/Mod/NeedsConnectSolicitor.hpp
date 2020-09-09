#ifndef BOSS_MOD_NEEDSCONNECTSOLICITOR_HPP
#define BOSS_MOD_NEEDSCONNECTSOLICITOR_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::NeedsConnectSolicitor
 *
 * @brief handles NeedsConnect messages.
 *
 * @desc Handles NeedsConnect messages by
 * soliciting connect candidates via
 * SolicitConnectCandidates messages,
 * and waiting for ProposeConnectCandidates,
 * then emitting individual RequestConnect.
 */
class NeedsConnectSolicitor {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	explicit
	NeedsConnectSolicitor(S::Bus& bus_);
	NeedsConnectSolicitor(NeedsConnectSolicitor&&);
	~NeedsConnectSolicitor();
};

}}

#endif /* !defined(BOSS_MOD_NEEDSCONNECTSOLICITOR_HPP) */
