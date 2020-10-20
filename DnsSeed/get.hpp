#ifndef DNSSEED_GET_HPP
#define DNSSEED_GET_HPP

#include<string>
#include<vector>

namespace Ev { template<typename a> class Io; }

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

/** DnsSeed::can_torify
 *
 * @brief check if we can use `torify` and add `+tcp` flag
 * to use a `tor` TCP connection to get seed via DNS.
 *
 * @return true if we can use `torify` with the given
 * seed and resolver.
 */
Ev::Io<bool> can_torify( std::string const& seed
		       , std::string const& resolver = "1.0.0.1"
		       );

/** DnsSeed::get
 *
 * @brief Get some number of connect proposals from
 * the given seed.
 * Requires the `dig` command.
 */
Ev::Io<std::vector<std::string>> get( std::string const& seed
				    , std::string const& resolver = "1.0.0.1"
				    , bool torify = false
				    );

}

#endif /* !defined(DNSSEED_GET_HPP) */
