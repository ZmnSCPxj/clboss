#ifndef DNSSEED_GET_HPP
#define DNSSEED_GET_HPP

#include<string>
#include<vector>

namespace Ev { template<typename a> class Io; }
namespace Ev { class ThreadPool; }

namespace DnsSeed {

/** DnsSeed::can_get
 *
 * @brief check if the get() function is useful.
 *
 * @return empty string if the get() function can be
 * used.
 * Otherwise, returns a human-readable reason why the
 * get() function cannot be used.
 */
Ev::Io<std::string> can_get();

/** DnsSeed::get
 *
 * @brief Get some number of connect proposals from
 * the given seed.
 * Requires the `dig` command.
 */
Ev::Io<std::vector<std::string>> get( std::string const& seed
				    , Ev::ThreadPool& threadpool
				    );

}

#endif /* !defined(DNSSEED_GET_HPP) */
