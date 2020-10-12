#ifndef BOSS_MOD_DOWSER_HPP
#define BOSS_MOD_DOWSER_HPP

#include"Ln/NodeId.hpp"

namespace Boss { namespace Mod { class Rpc; }}
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::Dowser
 *
 * @brief Determines the amount of plausible flow between two nodes.
 *
 * @desc This module responds to `Boss::Msg::RequestDowser`.
 */
class Dowser {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;
	Ln::NodeId self_id;

	class Run;

	void start();

public:
	Dowser() =delete;
	Dowser(Dowser&&) =delete;
	Dowser(Dowser const&) =delete;

	explicit
	Dowser( S::Bus& bus_
	      ) : bus(bus_)
		, rpc(nullptr)
		, self_id()
		{ start(); }
};

}}

#endif /* !defined(BOSS_MOD_DOWSER_HPP) */
