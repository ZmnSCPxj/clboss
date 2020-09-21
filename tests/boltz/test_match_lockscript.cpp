#undef NDEBUG
#include"Boltz/Detail/match_lockscript.hpp"
#include"Ripemd160/Hash.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Util/Str.hpp"
#include<assert.h>

int main() {
	auto res = bool();
	auto hash = Ripemd160::Hash();
	auto pubkey_hash = Secp256k1::PubKey();
	auto locktime = std::uint32_t();
	auto pubkey_locktime = Secp256k1::PubKey();

	auto test = [&](std::string const& s) {
		res = Boltz::Detail::match_lockscript
			( hash, pubkey_hash, locktime, pubkey_locktime
			, Util::Str::hexread(s)
			);
	};

	test("8201208763a9142c2d5441ef4a4469eae063941463e3c65ee926c088210207262fc331c1c845c6f8f7cca7a04ec3bdb09ef82cddac3eda2953c10bddd77967750395e809b17521030a47fa92352ac70161366dad4b45ad2a2fcbf5cef40fec43b29d33128ace529e68ac");
	assert(res);
	assert(hash == Ripemd160::Hash("2c2d5441ef4a4469eae063941463e3c65ee926c0"));
	assert(pubkey_hash == Secp256k1::PubKey("0207262fc331c1c845c6f8f7cca7a04ec3bdb09ef82cddac3eda2953c10bddd779"));
	assert(locktime == 649365);
	assert(pubkey_locktime == Secp256k1::PubKey("030a47fa92352ac70161366dad4b45ad2a2fcbf5cef40fec43b29d33128ace529e"));

	test("8201218763a9142c2d5441ef4a4469eae063941463e3c65ee926c088210207262fc331c1c845c6f8f7cca7a04ec3bdb09ef82cddac3eda2953c10bddd77967750395e809b17521030a47fa92352ac70161366dad4b45ad2a2fcbf5cef40fec43b29d33128ace529e68ac");
	assert(!res);

	test("8201208763a9142c2d5441ef4a4469eae063941463e3c65ee926c08821ff07262fc331c1c845c6f8f7cca7a04ec3bdb09ef82cddac3eda2953c10bddd77967750395e809b17521030a47fa92352ac70161366dad4b45ad2a2fcbf5cef40fec43b29d33128ace529e68ac");
	assert(!res);

	test("8201208763a9142c2d5441ef4a4469eae063941463e3c65ee926c088210207262fc331c1c845c6f8f7cca7a04ec3bdb09ef82cddac3eda2953c10bddd77967750295e8b17521030a47fa92352ac70161366dad4b45ad2a2fcbf5cef40fec43b29d33128ace529e68ac");
	assert(!res);

	test("8201208763a9142c2d5441ef4a4469eae063941463e3c65ee926c088210207262fc331c1c845c6f8f7cca7a04ec3bdb09ef82cddac3eda2953c10bddd77967750395e809b17521ff0a47fa92352ac70161366dad4b45ad2a2fcbf5cef40fec43b29d33128ace529e68ac");
	assert(!res);

	test("8201208763a9142c2d5441ef4a4469eae063941463e3c65ee926c088210207262fc331c1c845c6f8f7cca7a04ec3bdb09ef82cddac3eda2953c10bddd77967750395e809b17521030a47fa92352ac70161366dad4b45ad2a2fcbf5cef40fec43b29d33128ace529e68ac7551");

	return 0;
}
