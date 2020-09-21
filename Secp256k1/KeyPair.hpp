#ifndef SECP256K1_KEYPAIR_HPP
#define SECP256K1_KEYPAIR_HPP

#include<string>
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"

namespace Secp256k1 { class Random; }

namespace Secp256k1 {

/* A pair of privkey and pubkey which maintains the invariant
 * that the public key corresponds to the private key.
 */
class KeyPair {
private:
	Secp256k1::PrivKey sk;
	Secp256k1::PubKey pk;

public:
	KeyPair() : sk(), pk(sk) { }
	explicit KeyPair(Secp256k1::PrivKey const& sk_) : sk(sk_), pk(sk) { }
	explicit KeyPair(std::string const& sk_) : sk(sk_), pk(sk) { }
	explicit KeyPair(Secp256k1::Random& sk_) : sk(sk_), pk(sk) { }
	KeyPair(KeyPair const&) =default;
	KeyPair& operator=(KeyPair const&) =default;
	KeyPair(KeyPair&&) =default;
	KeyPair& operator=(KeyPair&&) =default;

	KeyPair& operator=(Secp256k1::PrivKey const& o) {
		sk = o;
		pk = Secp256k1::PubKey(sk);
		return *this;
	}

	Secp256k1::PrivKey const& priv() const { return sk; }
	Secp256k1::PubKey const& pub() const { return pk; }
};

}

#endif /* SECP256K1_KEYPAIR_HPP */
