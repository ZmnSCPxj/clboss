#include"Bitcoin/hash160.hpp"
#include"Ripemd160/Hash.hpp"
#include"Ripemd160/Hasher.hpp"
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"

namespace Bitcoin {

void hash160( Ripemd160::Hash& hash160, Sha256::Hash& sha256
	    , void const* p, std::size_t len
	    ) {
	sha256 = Sha256::fun(p, len);

	std::uint8_t buf[32];
	sha256.to_buffer(buf);

	auto r_hasher = Ripemd160::Hasher();
	r_hasher.feed(buf, sizeof(buf));
	hash160 = std::move(r_hasher).finalize();
}
Ripemd160::Hash hash160(void const* p, std::size_t len) {
	auto sha256 = Sha256::Hash();
	auto hash160_ = Ripemd160::Hash();
	hash160(hash160_, sha256, p, len);
	return hash160_;
}

}
