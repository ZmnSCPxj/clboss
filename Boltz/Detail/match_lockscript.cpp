#include"Boltz/Detail/match_lockscript.hpp"
#include"Ripemd160/Hash.hpp"
#include"Secp256k1/PubKey.hpp"
#include<iterator>

namespace {

/* Thrown by below if eof or unmatche.  */
struct Error { };

/* Reads from the given sequence, throws
 * Eof() if absent.  */
template<typename It>
class Reader {
private:
	It b;
	It e;
public:
	Reader(It b_, It e_) : b(b_), e(e_) { }

	typename std::iterator_traits<It>::reference
	get() {
		if (b == e)
			throw Error();
		return *(b++);
	}
	void expect(typename std::iterator_traits<It>::value_type v) {
		auto const& actual = get();
		if (actual != v)
			throw Error();
	}
	void expect_end() {
		if (b != e)
			throw Error();
	}
};

template<typename It>
Reader<It> make_reader(It b, It e) {
	return Reader<It>(b, e);
}

}

namespace Boltz { namespace Detail {

bool match_lockscript( Ripemd160::Hash& hash
		     , Secp256k1::PubKey& pubkey_hash
		     , std::uint32_t& locktime
		     , Secp256k1::PubKey& pubkey_locktime
		     , std::vector<std::uint8_t> const& script
		     ) {
	std::uint8_t buf[33];
	auto r = make_reader(script.begin(), script.end());
	/* Oh no, the try-if antipattern!
	 * This still ends up more succinct than using `if`
	 * over and over again, however.
	 */
	try {
		/* Template
		82 OP_SIZE
		01 20 push(32)
		87 OP_EQUAL
		63 OP_IF
		  a9 OP_HASH160
		  14 (20bytes) push(RIPEMD160(hash))
		  88 OP_EQUALVERIFY
		  21 (33bytes) push(pubkey_hash)
		67 OP_ELSE
		  75 OP_DROP
		  03 (3 bytes little-endian) push(locktime)
		  b1 OP_CHECKLOCKTIMEVERIFY
		  75 OP_DROP
		  21 (33 bytes) push(pubkey_locktime)
		68 OP_ENDIF
		ac OP_CHECKSIG
		*/
		r.expect(0x82);
		r.expect(0x01); r.expect(0x20);
		r.expect(0x87);
		r.expect(0x63);
		r.expect(0xa9);
		r.expect(0x14);
		for (auto i = std::size_t(0); i < 20; ++i)
			buf[i] = r.get();
		hash.from_buffer(buf);
		r.expect(0x88);
		r.expect(0x21);
		for (auto i = std::size_t(0); i < 33; ++i)
			buf[i] = r.get();
		pubkey_hash = Secp256k1::PubKey::from_buffer(buf);
		r.expect(0x67);
		r.expect(0x75);
		/* Always 3 bytes?
		 * 1->2 bytes would only work for very young blockchains.
		 * 3 bytes would work for blockchains of at least 65,536
		 * to 16,777,215 blocks, which is a fairly large range.
		 */
		r.expect(0x03);
		for (auto i = std::size_t(0); i < 3; ++i)
			buf[i] = r.get();
		locktime = (std::uint32_t(buf[0]) << 0)
			 | (std::uint32_t(buf[1]) << 8)
			 | (std::uint32_t(buf[2]) << 16)
			 ;
		r.expect(0xb1);
		r.expect(0x75);
		r.expect(0x21);
		for (auto i = std::size_t(0); i < 33; ++i)
			buf[i] = r.get();
		pubkey_locktime = Secp256k1::PubKey::from_buffer(buf);
		r.expect(0x68);
		r.expect(0xac);

		r.expect_end();
		return true;
	} catch (Error const&) {
		/* Some problem.  */
		return false;
	} catch (Secp256k1::InvalidPubKey const&) {
		/* Invalid pubkey.  */
		return false;
	}
}

}}
