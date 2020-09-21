#include<sodium/randombytes.h>
#include"Secp256k1/Random.hpp"
#include"Util/make_unique.hpp"

namespace Secp256k1 {

class Random::Impl {
private:
	unsigned int num;
	std::uint8_t buffer[64];

public:
	Impl() : num(64) { }
	std::uint8_t get() {
		if (num >= 64) {
			randombytes_buf(buffer, 64);
			num = 0;
		}
		return buffer[num++];
	}
};

Random::Random() : pimpl(Util::make_unique<Impl>()) { }
Random::~Random() { }

std::uint8_t Random::get() {
	return pimpl->get();
}

}

