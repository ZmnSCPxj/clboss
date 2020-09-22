#ifndef BITCOIN_SIGHASH_HPP
#define BITCOIN_SIGHASH_HPP

#include<cstddef>
#include<cstdint>
#include<stdexcept>
#include<vector>

namespace Bitcoin { class Tx; }
namespace Ln { class Amount; }
namespace Sha256 { class Hash; }

namespace Bitcoin {

enum SighashFlags
{ SIGHASH_ALL = 1
, SIGHASH_NONE = 2
, SIGHASH_SINGLE = 3
, SIGHASH_ANYONECANPAY = 0x80
};

struct InvalidSighash : public std::invalid_argument {
	InvalidSighash() =delete;
	InvalidSighash(std::string const& msg)
		: std::invalid_argument(
			std::string("Bitcoin::InvalidSighash: ") + msg
		  ) { }
};

/** Bitcoin::sighash
 *
 * @brief computes the sighash for the
 * given transaction, being signed with
 * the given flags, and signing for the
 * input nIn.
 *
 * @desc throws `Bitcoin::InvalidSighash`
 * if the given `flags` is not recognized,
 * or if `nIn` is out-of-range.
 *
 * The given `scriptCode` should consider
 * `OP_CODESEPERATOR`.
 * For P2WPKH it should be the constructed
 * script "1976a914{20-byte-pubkey-hash}88ac".
 * For P2WSH it should be the `witnessScript`
 * whose hash is what is committed in the
 * `scriptPubKey`, and which is the top of the
 * witness stack.
 *
 * This is strictly for SegWit sighash
 * algorithm.
 */
Sha256::Hash
sighash( Bitcoin::Tx const& tx
       , SighashFlags flags
       , std::size_t nIn
       , Ln::Amount amount
       , std::vector<std::uint8_t> const& scriptCode
       );

}

#endif /* !defined(BITCOIN_SIGHASH_HPP) */
