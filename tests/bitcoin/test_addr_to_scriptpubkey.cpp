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

}

int main() {
	test_addr( "bc1qg430dgu75qvphu8nn3kvcp3yg2km2tavpc8lqs"
		 , "00144562f6a39ea0181bf0f39c6ccc062442adb52fac"
		 );
	test_addr( "bc1qdzrwd6ygu0npsvld2ah9d3yvypekakzkau758t"
		 , "00146886e6e888e3e61833ed576e56c48c20736ed856"
		 );
	/* TODO: P2WSH tests.  */

	return 0;
}
