#ifndef BOSS_MOD_BLOCKTRACKER_HPP
#define BOSS_MOD_BLOCKTRACKER_HPP

#include<cstdint>
#include<string>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::BlockTracker
 *
 * @brief simple module to emit a Boss::Msg::Block message
 * at each block processed by lightningd.
 */
class BlockTracker {
private:
	S::Bus& bus;

	std::uint32_t height;

	Ev::Io<bool> fail(std::string);
	Ev::Io<void> loop(Boss::Mod::Rpc&);
	void start();

public:
	explicit
	BlockTracker(S::Bus& bus_) : bus(bus_) { start(); }
};

}}

#endif /* !defined(BOSS_MOD_BLOCKTRACKER_HPP) */
