#undef NDEBUG
#include"Bitcoin/addr_to_scriptPubKey.hpp"
#include"Util/Str.hpp"
#include<assert.h>
#include<string>

namespace {

void test_addr( char const* addr
	      , char const* hexscript
	      ) {
	auto script = Bitcoin::addr_to_scriptPubKey(addr);
	assert(script == Util::Str::hexread(hexscript));
}

void test_fail( char const* bad_addr) {
	auto flag = bool();
	try {
		Bitcoin::addr_to_scriptPubKey(bad_addr);
		flag = false;
	} catch (Bitcoin::UnknownAddrType const& _) {
		flag = true;
	}
	assert(flag);
}

}

int main() {
	test_addr( "bc1qg430dgu75qvphu8nn3kvcp3yg2km2tavpc8lqs"
		 , "00144562f6a39ea0181bf0f39c6ccc062442adb52fac"
		 );
	test_addr( "bc1qdzrwd6ygu0npsvld2ah9d3yvypekakzkau758t"
		 , "00146886e6e888e3e61833ed576e56c48c20736ed856"
		 );
	/* From BIP-173.  */
	test_addr( "BC1QW508D6QEJXTDG4Y5R3ZARVARY0C5XW7KV8F3T4"
		 , "0014751e76e8199196d454941c45d1b3a323f1433bd6"
		 );
	test_addr( "tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3q0sl5k7"
		 , "00201863143c14c5166804bd19203356da136c985678cd4d27a1b8c6329604903262"
		 );
	/* Removed --- incoming Taproot softfork specifies only 32-byte
	 * segwit v1 addresses, we enforce that version 1 segwit have
	 * 32 bytes exactly.  */
#if 0
	test_addr( "bc1pw508d6qejxtdg4y5r3zarvary0c5xw7kw508d6qejxtdg4y5r3zarvary0c5xw7k7grplx"
		 , "5128751e76e8199196d454941c45d1b3a323f1433bd6751e76e8199196d454941c45d1b3a323f1433bd6"
		 );
#endif
	test_addr( "BC1SW50QA3JX3S"
		 , "6002751e"
		 );
	test_addr( "bc1zw508d6qejxtdg4y5r3zarvaryvg6kdaj"
		 , "5210751e76e8199196d454941c45d1b3a323"
		 );
	test_addr( "tb1qqqqqp399et2xygdj5xreqhjjvcmzhxw4aywxecjdzew6hylgvsesrxh6hy"
		 , "0020000000c4a5cad46221b2a187905e5266362b99d5e91c6ce24d165dab93e86433"
		 );

	/* Invalid addresses from BIP-173.  */
	test_fail("tc1qw508d6qejxtdg4y5r3zarvary0c5xw7kg3g4ty");
#if 0
	// TODO: Implement checksum check.
	test_fail("bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t5");
#endif
	test_fail("BC13W508D6QEJXTDG4Y5R3ZARVARY0C5XW7KN40WF2");
	test_fail("bc1rw5uspcuh");
	test_fail("bc10w508d6qejxtdg4y5r3zarvary0c5xw7kw508d6qejxtdg4y5r3zarvary0c5xw7kw5rljs90");
	test_fail("BC1QR508D6QEJXTDG4Y5R3ZARVARYV98GJ9P");
#if 0
	// TODO: Implement consistent case check.
	test_fail("tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3q0sL5k7");
#endif
	test_fail("bc1zw508d6qejxtdg4y5r3zarvaryvqyzf3du");
	test_fail("tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3pjxtptv");
	test_fail("bc1gmk9yu");

	return 0;
}
