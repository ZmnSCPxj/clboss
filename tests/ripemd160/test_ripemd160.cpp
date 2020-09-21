#undef NDEBUG
#include"Boss/random_engine.hpp"
#include"Ripemd160/Hash.hpp"
#include"Ripemd160/Hasher.hpp"
#include"Util/Str.hpp"
#include<assert.h>
#include<random>

namespace {

void test_ripemd160(std::string const& s, std::string const& hash) {
	/* Test in one big go.  */
	{
		auto hasher = Ripemd160::Hasher();
		hasher.feed(&s[0], s.size());
		assert(hasher.get() == Ripemd160::Hash(hash));
		assert(std::string(std::move(hasher).finalize()) == hash);
	}

	/* Test chopped up.  */
	{
		auto hasher = Ripemd160::Hasher();
		auto ptr = &s[0];
		auto len = s.size();
		while (len > 0) {
			auto dist = std::uniform_int_distribution<std::size_t>(
				1, len
			);
			auto thislen = dist(Boss::random_engine);
			hasher.feed(ptr, thislen);
			ptr += thislen;
			len -= thislen;
		}
		assert(std::move(hasher).finalize() == Ripemd160::Hash(hash));
	}
}

}

int main() {
	/* From Bitcoin test vectors.  */
	test_ripemd160("", "9c1185a5c5e9fc54612808977ee8f548b2258d31");
	test_ripemd160("abc", "8eb208f7e05d987a9b044a8e98c6b087f15a0bfc");
	test_ripemd160( "For this sample, this 63-byte string will be used as input data"
		      , "de90dbfee14b63fb5abf27c2ad4a82aaa5f27a11"
		      );
	test_ripemd160( "This is exactly 64 bytes long, not counting the terminating byte"
		      , "eda31d51d3a623b81e19eb02e24ff65d27d67b37"
		      );
	test_ripemd160( std::string(1000000, 'a')
		      , "52783243c1697bdbe16d37f97f68f08325dc1528"
		      );

	return 0;
}
