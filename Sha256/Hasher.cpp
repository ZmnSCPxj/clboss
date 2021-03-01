#include"Sha256/Hash.hpp"
#include"Sha256/Hasher.hpp"
#include"Util/make_unique.hpp"
#include<crypto/sha256.h>
#include<sodium/utils.h>

namespace Sha256 {

class Hasher::Impl {
private:
	CSHA256 s;

public:
	Impl() { }
	~Impl() {
		sodium_memzero(&s, sizeof(s));
	}
	Impl(Impl const&) =default;

	void feed(void const* p, std::size_t len) {
		s.Write((const unsigned char*) p, len);
	}
	void finalize(std::uint8_t buff[32]) {
		s.Finalize((unsigned char *) buff);
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
