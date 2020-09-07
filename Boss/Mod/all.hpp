#ifndef BOSS_MOD_ALL_HPP
#define BOSS_MOD_ALL_HPP

#include<memory>
#include<ostream>

namespace Ev { class ThreadPool; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** Boss::Mod::all
 *
 * @brief Constructs all the modules of the boss.
 * Returns a shared pointer to an object that
 * cleans up all modules on destruction.
 */
std::shared_ptr<void> all( std::ostream& cout
			 , S::Bus& bus
			 , Ev::ThreadPool& threadpool
			 );

}}

#endif /* !defined(BOSS_MOD_ALL_HPP) */
