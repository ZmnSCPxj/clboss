#ifndef BOSS_MOD_CONNECTFINDERBYHARDCODE_HPP
#define BOSS_MOD_CONNECTFINDERBYHARDCODE_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ConnectFinderByHardcode
 *
 * @brief provides some possible connection
 * candidates from a hardcoded list.
 */
class ConnectFinderByHardcode {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	ConnectFinderByHardcode() =delete;
	ConnectFinderByHardcode(S::Bus& bus);
	ConnectFinderByHardcode(ConnectFinderByHardcode&&);
	~ConnectFinderByHardcode();
};

}}

#endif /* BOSS_MOD_CONNECTFINDERBYHARDCODE_HPP */
