#ifndef BITCOIN_HASH160_HPP
#define BITCOIN_HASH160_HPP

#include<cstdint>

namespace Ripemd160 { class Hash; }
namespace Sha256 { class Hash; }

namespace Bitcoin {

/** Bitcoin::hash160
 *
 * @brief hashes the input by SHA256, then
 * hashes the SHA256 hash by RIPEMD160.
 *
 * @desc There are two variants, one which
 * just returns the RIPEMD160, and the
 * other which also outputs the intermediate
 * SHA256.
 */
Ripemd160::Hash hash160(void const* p, std::size_t len);
void hash160( Ripemd160::Hash& hash160, Sha256::Hash& sha256
	    , void const* p, std::size_t len
	    );

}

#endif /* BITCOIN_HASH160_HPP */
