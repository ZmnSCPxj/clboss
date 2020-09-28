#ifndef BOLTZ_DETAIL_COMPUTE_PREIMAGE_HPP
#define BOLTZ_DETAIL_COMPUTE_PREIMAGE_HPP

namespace Ln { class Preimage; }
namespace Secp256k1 { class SignerIF; }

namespace Boltz { namespace Detail {

/** Boltz::Detail::compute_preimage
 *
 * @brief computes the actual preimage to use,
 * based on a base preimage and the signer
 * privkey.
 *
 * @desc this ensures that the preimage in the
 * database is not the actual preimage that is
 * used when dealing with the Boltz instance,
 * so that even if a developer runs a Boltz
 * instance and is given a database to debug,
 * there is no risk of loss to the user who
 * gave the database.
 */
Ln::Preimage compute_preimage( Secp256k1::SignerIF& signer
			     , Ln::Preimage const& base_preimage
			     );

}}

#endif /* !defined(BOLTZ_DETAIL_COMPUTE_PREIMAGE_HPP) */
