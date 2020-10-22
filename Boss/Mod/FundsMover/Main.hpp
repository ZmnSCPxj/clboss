#ifndef BOSS_MOD_FUNDSMOVER_MAIN_HPP
#define BOSS_MOD_FUNDSMOVER_MAIN_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod { namespace FundsMover {

/** class Boss::Mod::FundsMover::Main
 *
 * @brief primary holding module for `FundsMover` module.
 */
class Main {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Main() =delete;

	Main(Main&&);
	~Main();

	explicit
	Main(S::Bus&);
};

}}}

#endif /* !defined(BOSS_MOD_FUNDSMOVER_MAIN_HPP) */
