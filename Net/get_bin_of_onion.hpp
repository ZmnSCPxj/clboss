#ifndef NET_GET_BIN_OF_ONION_HPP
#define NET_GET_BIN_OF_ONION_HPP

#include<cstdint>
#include<functional>
#include<string>

namespace Net {

typedef std::uint64_t IPBin;

/** Net::get_bin_of_onion
 *
 * @brief common code to invent a bin for onion addresses.
 */
IPBin get_bin_of_onion( std::function<void(std::uint8_t hash[32])> tweak_hash
		      , std::string const& onion
		      );

}

#endif /* !defined(NET_GET_BIN_OF_ONION_HPP) */
