#include<assert.h>
#include<iomanip>
#include<secp256k1.h>
#include<sodium/utils.h>
#include<sstream>
#include<string>
#include<string.h>
#include<utility>
#include"Secp256k1/Detail/context.hpp"
#include"Secp256k1/G.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Util/Str.hpp"
#include"Util/make_unique.hpp"

using Secp256k1::Detail::context;

namespace Secp256k1 {

class PubKey::Impl {
public:
	secp256k1_pubkey key;

	explicit Impl(Secp256k1::PrivKey const& sk) {
		auto res = secp256k1_ec_pubkey_create( context.get()
						     , &key
						     , sk.key
						     );
		/* The private key should have been verified.  */
		assert(res == 1);
	}
	Impl(secp256k1_context_struct *ctx, std::uint8_t buffer[33]) {
		auto res = secp256k1_ec_pubkey_parse( ctx
						    , &key
						    , buffer
						    , 33
						    );
		if (!res)
			throw InvalidPubKey();
	}
	Impl(std::uint8_t const buffer[33]) {
		auto res = secp256k1_ec_pubkey_parse( context.get()
						    , &key
						    , buffer
						    , 33
						    );
		if (!res)
			throw InvalidPubKey();
	}
	Impl() { }
	Impl(Impl const& o) {
		key = o.key;
	}

	void negate() {
		auto res = secp256k1_ec_pubkey_negate(context.get(), &key);
		assert(res == 1);
	}
	void add(Impl const& o) {
		secp256k1_pubkey tmp;

		secp256k1_pubkey const* keys[2] = {&o.key, &key};
		auto res = secp256k1_ec_pubkey_combine( context.get()
						      , &tmp
						      , keys
						      , 2
						      );
		/* FIXME: use a backtrace-prserving exception.  */
		if (!res)
			throw std::out_of_range(
				"Secp256k1::PubKey::operatoor+=: "
				"result of adding PubKey out-of-range"
			);

		key = tmp;
	}
	void mul(Secp256k1::PrivKey const& sk) {
		/* Construct temporary. */
		auto tmp = key;

		auto res = secp256k1_ec_pubkey_tweak_mul( context.get()
							, &tmp
							, sk.key
							);
		if (!res)
			throw std::out_of_range(
				"Secp256k1::PubKey::operatoor*=: "
				"result of multiplying PrivKey out-of-range"
			);
		/* swap. */
		key = tmp;
	}

	bool equal(Impl const& o) const {
		std::uint8_t a[33];
		std::uint8_t b[33];
		size_t asize = sizeof(a);
		size_t bsize = sizeof(b);

		auto resa = secp256k1_ec_pubkey_serialize( context.get()
							 , a
							 , &asize
							 , &key
							 , SECP256K1_EC_COMPRESSED
							 );
		assert(resa == 1);
		assert(asize == sizeof(a));

		auto resb = secp256k1_ec_pubkey_serialize( context.get()
							 , b
							 , &bsize
							 , &o.key
							 , SECP256K1_EC_COMPRESSED
							 );
		assert(resb == 1);
		assert(bsize == sizeof(b));

		return sodium_memcmp(a, b, sizeof(a)) == 0;
	}

	void dump(std::ostream& os) {
		std::uint8_t a[33];
		size_t asize = sizeof(a);

		auto resa = secp256k1_ec_pubkey_serialize( context.get()
							 , a
							 , &asize
							 , &key
							 , SECP256K1_EC_COMPRESSED
							 );
		assert(resa == 1);
		assert(asize == sizeof(a));

		for (auto i = 0; i < 33; ++i) {
			os << Util::Str::hexbyte(a[i]);
		}
	}

	void to_buffer(std::uint8_t buffer[33]) const {
		size_t size = 33;
		auto resa = secp256k1_ec_pubkey_serialize( context.get()
							 , buffer
							 , &size
							 , &key
							 , SECP256K1_EC_COMPRESSED
							 );
		assert(resa == 1);
		assert(size == 33);
	}
};

PubKey::PubKey()
	: pimpl(Util::make_unique<Impl>(*G.pimpl)) { }

PubKey::PubKey(secp256k1_context_struct *ctx, std::uint8_t buffer[33])
	: pimpl(Util::make_unique<Impl>(ctx, buffer)) {}
PubKey::PubKey(std::uint8_t const buffer[33])
	: pimpl(Util::make_unique<Impl>(buffer)) {}

PubKey::PubKey(std::string const& s) {
	auto buf = Util::Str::hexread(s);
	if (buf.size() != 33)
		/* FIXME: Use a specific invalid-public-key failure.  */
		throw InvalidPubKey();
	pimpl = Util::make_unique<Impl>(&buf[0]);
}
PubKey::operator std::string() const {
	auto os = std::ostringstream();
	pimpl->dump(os);
	return os.str();
}

PubKey::PubKey(Secp256k1::PrivKey const& sk)
	: pimpl(Util::make_unique<Impl>(sk)) {}

PubKey::PubKey(PubKey const& o)
	: pimpl(Util::make_unique<Impl>(*o.pimpl)) { }
PubKey::PubKey(PubKey&& o) {
	auto mine = Util::make_unique<Impl>();
	std::swap(pimpl, mine);
	std::swap(pimpl, o.pimpl);
}
PubKey::~PubKey() { }

void const* PubKey::get_key() const {
	return &pimpl->key;
}

void PubKey::negate() {
	pimpl->negate();
}

PubKey& PubKey::operator+=(PubKey const& o) {
	pimpl->add(*o.pimpl);
	return *this;
}
PubKey& PubKey::operator*=(PrivKey const& o) {
	pimpl->mul(o);
	return *this;
}

bool PubKey::operator==(PubKey const& o) const {
	return pimpl->equal(*o.pimpl);
}

void PubKey::to_buffer(std::uint8_t buffer[33]) const {
	pimpl->to_buffer(buffer);
}

}

std::ostream& operator<<(std::ostream& os, Secp256k1::PubKey const& pk) {
	pk.pimpl->dump(os);
	return os;
}
