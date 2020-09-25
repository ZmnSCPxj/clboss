#ifndef SECP256K1_SIGNATURE_HPP
#define SECP256K1_SIGNATURE_HPP

#include<cstdint>
#include<stdexcept>
#include<string>
#include<vector>

namespace Secp256k1 { class PrivKey; }
namespace Secp256k1 { class PubKey; }
namespace Sha256 { class Hash; }

namespace Secp256k1 {

class BadSignatureEncoding : public std::invalid_argument {
public:
	BadSignatureEncoding()
		: std::invalid_argument("Bad signature encoding")
		{ }
};

class Signature {
private:
	std::uint8_t data[64];

	Signature( Secp256k1::PrivKey const&
		 , Sha256::Hash const&
		 );
	Signature(std::uint8_t const buffer[64]);

	bool sig_has_low_r() const;

public:
	Signature();
	Signature(Signature const&) =default;
	Signature& operator=(Signature const&) =default;

	explicit Signature(std::string const&);

	static
	Signature from_buffer(std::uint8_t buffer[64]) {
		return Signature(buffer);
	}

	void to_buffer(std::uint8_t buffer[64]) const;

	/* Check if the signature is valid for the given pubkey
	 * and message hash.
	 * We impose the low-s rule.
	 */
	bool valid( Secp256k1::PubKey const& pk
		  , Sha256::Hash const& m
		  ) const;

	/* Create a valid signature for the given privkey and
	 * message hash.
	 */
	static
	Signature create( Secp256k1::PrivKey const& sk
			, Sha256::Hash const& m
			) {
		return Signature(sk, m);
	}

	/** Secp256k1::Signature::der_encode
	 *
	 * @brief Return the DER-encoding of the signature.
	 */
	std::vector<std::uint8_t> der_encode() const;
	/** Secp256k1::Signature::der_decode
	 *
	 * @brief Return the signature from the der
	 * decoding.
	 */
	static
	Signature der_decode(std::vector<std::uint8_t> const& d);
};

}

#endif /* SECP256K1_SIGNATURE_HPP */
