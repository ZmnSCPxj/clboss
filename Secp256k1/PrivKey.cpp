#include<assert.h>
#include<iomanip>
#include<secp256k1.h>
#include<sodium/utils.h>
#include<sstream>
#include<stdexcept>
#include<string>
#include<string.h>
#include"Secp256k1/Detail/context.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/Random.hpp"
#include"Util/Str.hpp"

using Secp256k1::Detail::context;

namespace  Secp256k1 {

PrivKey::PrivKey(std::uint8_t const key_[32]) {
	if (!secp256k1_ec_seckey_verify(context.get(), key_))
		throw InvalidPrivKey();
	memcpy(key, key_, 32);
}

PrivKey::PrivKey() {
	/* 1 */
	for (auto i = 0; i < 31; ++i)
		key[i] = 0;
	key[31] = 1;
}

PrivKey::PrivKey(std::string const& s) {
	auto buf = Util::Str::hexread(s);
	if (buf.size() != 32)
		throw InvalidPrivKey();
	if (!secp256k1_ec_seckey_verify(context.get(), &buf[0]))
		throw InvalidPrivKey();
	memcpy(key, &buf[0], 32);
}
PrivKey::operator std::string() const {
	auto os = std::ostringstream();
	os << *this;
	return os.str();
}

PrivKey::PrivKey(Secp256k1::Random& rand) {
	do {
		for (auto i = 0; i < 32; ++i) {
			key[i] = rand.get();
		}
	} while (!secp256k1_ec_seckey_verify(context.get(), key));
}

PrivKey::PrivKey(PrivKey const& o) {
	memcpy(key, o.key, 32);
}

PrivKey::~PrivKey() {
	sodium_memzero(key, sizeof(key));
}

PrivKey& PrivKey::negate() {
	auto res = secp256k1_ec_privkey_negate(context.get(), key);
	assert(res == 1);
	return *this;
}

PrivKey& PrivKey::operator+=(PrivKey const& o) {
	auto res = secp256k1_ec_privkey_tweak_add(context.get(), key, o.key);
	/* FIXME: Use a backtrace-catching exception. */
	if (!res)
		throw std::out_of_range("Secp256k1::PrivKey += out-of-range");
	return *this;
}

PrivKey& PrivKey::operator*=(PrivKey const& o) {
	auto res = secp256k1_ec_privkey_tweak_mul(context.get(), key, o.key);
	/* FIXME: Use a backtrace-catching exception. */
	if (!res)
		throw std::out_of_range("Secp256k1::PrivKey += out-of-range");
	return *this;
}

bool PrivKey::operator==(PrivKey const& o) const {
	return 0 == sodium_memcmp(key, o.key, sizeof(key));
}

}

std::ostream& operator<<(std::ostream& os, Secp256k1::PrivKey const& sk) {
	for (auto i = 0; i < 32; ++i)
		os << Util::Str::hexbyte(sk.key[i]);
	return os;
}
