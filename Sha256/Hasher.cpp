#include"Sha256/Hash.hpp"
#include"Sha256/Hasher.hpp"
#include"Util/make_unique.hpp"
#include<sodium/crypto_hash_sha256.h>
#include<sodium/utils.h>

namespace Sha256 {

class Hasher::Impl {
private:
	crypto_hash_sha256_state s;

public:
	Impl() {
		crypto_hash_sha256_init(&s);
	}
	~Impl() {
		sodium_memzero(&s, sizeof(s));
	}
	Impl(Impl const&) =default;

	void feed(void const* p, std::size_t len) {
		crypto_hash_sha256_update(&s, (unsigned char const*) p, len);
	}
	void finalize(std::uint8_t buff[32]) {
		crypto_hash_sha256_final(&s, buff);
	}
};

Hasher::Hasher() : pimpl(Util::make_unique<Impl>()) { }
Hasher::Hasher(Hasher&&) =default;
Hasher::~Hasher() =default;
Hasher::Hasher(Hasher const& o
	      ) : pimpl(Util::make_unique<Impl>(*o.pimpl)) { }

Hasher& Hasher::operator=(Hasher&&) =default;
Hasher& Hasher::operator=(Hasher const& i) {
	*this = Hasher(i);
	return *this;
}

Hasher::operator bool() const {
	return !!pimpl;
}
void Hasher::feed(void const* p, std::size_t len) {
	return pimpl->feed(p, len);
}

Sha256::Hash Hasher::finalize()&& {
	std::uint8_t buff[32];
	pimpl->finalize(buff);
	pimpl = nullptr;

	auto tmp = Sha256::Hash(buff);
	sodium_memzero(buff, sizeof(buff));

	return tmp;
}
Sha256::Hash Hasher::get() const {
	/* Copies are expensive!!  */
	auto tmp = *this;
	return std::move(tmp).finalize();
}

}
