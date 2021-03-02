#include"Ripemd160/Hash.hpp"
#include"Ripemd160/Hasher.hpp"
#include"Util/make_unique.hpp"
#include<basicsecure.h>
#include<crypto/ripemd160.h>

namespace Ripemd160 {

class Hasher::Impl {
private:
	CRIPEMD160 hasher;

public:
	Impl() : hasher() { }
	~Impl() {
		basicsecure_clear(&hasher, sizeof(hasher));
	}
	void feed(void const* p, std::size_t size) {
		hasher.Write((unsigned char const*) p, size);
	}
	void finalize(unsigned char buf[20]) {
		hasher.Finalize(buf);
	}
};

Hasher::Hasher() : pimpl(Util::make_unique<Impl>()) { }
Hasher::Hasher(Hasher&&) =default;
Hasher::~Hasher() =default;
Hasher::Hasher( Hasher const& o
	      ) : pimpl(Util::make_unique<Impl>(*o.pimpl)) { }

Hasher::operator bool() const {
	return !!pimpl;
}
void Hasher::feed(void const* p, std::size_t len) {
	return pimpl->feed(p, len);
}
Ripemd160::Hash Hasher::finalize()&& {
	unsigned char buf[20];
	pimpl->finalize(buf);
	pimpl = nullptr;

	auto tmp = Ripemd160::Hash(buf);
	basicsecure_clear(buf, sizeof(buf));
	return tmp;
}
Ripemd160::Hash Hasher::get() const {
	/* Copies are expensive!  */
	return Hasher(*this).finalize();
}

}
