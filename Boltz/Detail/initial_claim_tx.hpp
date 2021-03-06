#ifndef BOLTZ_DETAIL_INITIAL_CLAIM_TX_HPP
#define BOLTZ_DETAIL_INITIAL_CLAIM_TX_HPP

#include<cstdint>
#include<string>
#include<vector>

namespace Bitcoin { class Tx; }
namespace Bitcoin { class TxId; }
namespace Ln { class Amount; }
namespace Ln { class Preimage; }
namespace Secp256k1 { class PrivKey; }
namespace Secp256k1 { class SignerIF; }

namespace Boltz { namespace Detail {

/** Boltz::Detail::initial_claim_tx
 *
 * @brief creates the initial claim transaction
 * for a reverse submarine (offchain-to-onchain)
 * swap.
 */
void
initial_claim_tx( Bitcoin::Tx& claim_tx /* written by function */
		, Ln::Amount& claim_tx_fees /* written by function */

		/* Arguments to the function.  */

		/* Common: feerate and blockheight.  */
		, std::uint32_t feerate_perkw
		, std::uint32_t blockheight

		/* Details of the input.  */
		, Bitcoin::TxId const& lockup_txid
		, std::uint32_t lockup_outnum
		, Ln::Amount lockup_amount

		/* Witness details.  */
		, Secp256k1::SignerIF& signer
		, Secp256k1::PrivKey const& tweak
		, Ln::Preimage const& preimage
		, std::vector<std::uint8_t> const& witnessScript

		/* Output address.  */
		, std::string const& output_addr
		);

}}

#endif /* !defined(BOLTZ_DETAIL_INITIAL_CLAIM_TX_HPP) */
