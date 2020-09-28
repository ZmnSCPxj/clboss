#include"Boltz/Detail/compute_preimage.hpp"
#include"Ln/Preimage.hpp"
#include"Sha256/Hash.hpp"
#include"Secp256k1/SignerIF.hpp"

namespace Boltz { namespace Detail {

Ln::Preimage compute_preimage( Secp256k1::SignerIF& signer
			     , Ln::Preimage const& base_preimage
			     ) {
	std::uint8_t buf[32];
	base_preimage.to_buffer(buf);
	auto actual_preimage_h = signer.get_privkey_salted_hash(buf);
	actual_preimage_h.to_buffer(buf);

	auto actual_preimage = Ln::Preimage();
	actual_preimage.from_buffer(buf);
	return actual_preimage;
}

}}
