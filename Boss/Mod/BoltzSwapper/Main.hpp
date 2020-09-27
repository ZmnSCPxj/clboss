#ifndef BOSS_MOD_BOLTZSWAPPER_MAIN_HPP
#define BOSS_MOD_BOLTZSWAPPER_MAIN_HPP

#include<memory>

namespace Ev { class ThreadPool; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace BoltzSwapper {

/** class Boss::Mod::BoltzSwapper::Main
 *
 * @brief main module for interfacing with Boltz
 * instances.
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

	Main(S::Bus&, Ev::ThreadPool&);
};

}}}

#endif /* !defined(BOSS_MOD_BOLTZSWAPPER_MAIN_HPP) */
