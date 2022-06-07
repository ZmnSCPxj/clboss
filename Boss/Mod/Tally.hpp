#ifndef BOSS_MOD_TALLY_HPP
#define BOSS_MOD_TALLY_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::Tally
 *
 * @brief Module to keep track of some msat values.
 */
class Tally {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Tally() =delete;
	Tally(Tally const&) =delete;

	Tally(Tally&&);
	~Tally();

	explicit
	Tally(S::Bus&);
};

}}

#endif /* BOSS_MOD_TALLY_HPP */
