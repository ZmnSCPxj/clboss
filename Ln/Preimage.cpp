#include"Bitcoin/hash160.hpp"
#include"Ln/Preimage.hpp"
#include"Ripemd160/Hash.hpp"
#include"Ripemd160/Hasher.hpp"
#include"Secp256k1/Random.hpp"
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"
#include"Util/Str.hpp"
#include<sodium/utils.h>
#include<stdexcept>

namespace {

std::uint8_t const zero[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

}

namespace Ln {

Preimage::Preimage(Secp256k1::Random& r) : pimpl(std::make_shared<Impl>()) {
	for (auto i = 0; i < 32; ++i)
		pimpl->data[i] = r.get();
}
bool Preimage::valid_string(std::string const& s) {
	return s.size() == 64
	    && Util::Str::ishex(s)
	     ;
}
Preimage::Preimage(std::string const& s) : pimpl(std::make_shared<Impl>()) {
	auto buf = Util::Str::hexread(s);
	if (buf.size() != 32)
		throw std::invalid_argument("Ln::Preimage: wrong size.");
	for (auto i = 0; i < 32; ++i)
		pimpl->data[i] = buf[i];
}

Preimage::operator std::string() const {
	if (!pimpl)
		return "00000000000000000000000000000000000000000000000000000";
	return Util::Str::hexdump(pimpl->data, 32);
}

bool Preimage::operator==(Preimage const& o) const {
	auto a = pimpl ? pimpl->data : zero;
	auto b = o.pimpl ? o.pimpl->data : zero;
	return 0 == sodium_memcmp(a, b, 32);
}
Preimage::operator bool() const {
	if (!pimpl)
		return false;
	return 0 != sodium_memcmp(pimpl->data, zero, 32);
}

Sha256::Hash Preimage::sha256() const {
	return Sha256::fun(pimpl->data, 32);
}
Ripemd160::Hash Preimage::hash160() const {
	return Bitcoin::hash160(pimpl->data, 32);
}

}

