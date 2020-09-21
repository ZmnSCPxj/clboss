#ifndef BOLTZ_DETAIL_MATCH_LOCKSCRIPT_HPP
#define BOLTZ_DETAIL_MATCH_LOCKSCRIPT_HPP

#include<cstdint>
#include<vector>

namespace Ripemd160 { class Hash; }
namespace Secp256k1 { class PubKey; }

namespace Boltz { namespace Detail {

/** Boltz::Detail::match_lockscript
 *
 * @brief determines if the given supposed SCRIPT
 * matches the expected lockscript from a proper
 * BOLTZ instance.
 */
bool match_lockscript( Ripemd160::Hash& hash
		     , Secp256k1::PubKey& pubkey_hash
		     , std::uint32_t& locktime
		     , Secp256k1::PubKey& pubkey_locktime
		     , std::vector<std::uint8_t> const& script
		     );

}}

#endif /* !defined(BOLTZ_DETAIL_MATCH_LOCKSCRIPT_HPP) */
