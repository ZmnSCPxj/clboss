#undef NDEBUG
#include"Bitcoin/Tx.hpp"
#include"Bitcoin/TxId.hpp"
#include"Util/Str.hpp"
#include<assert.h>
#include<sstream>

#include<iostream>

namespace {

Bitcoin::Tx test_tx(char const* hextx, char const* hextxid) {
	auto tx_bytes = Util::Str::hexread(hextx);
	auto tx_s = std::string(tx_bytes.begin(), tx_bytes.end());
	auto is = std::istringstream(std::move(tx_s));

	auto tx = Bitcoin::Tx();
	is >> tx;

	assert(tx.get_txid() == Bitcoin::TxId(hextxid));

	auto os = std::ostringstream();
	os << tx;
	tx_s = os.str();
	assert(Util::Str::hexdump(&tx_s[0], tx_s.size()) == hextx);

	return tx;
}

}

int main() {
	auto tx = Bitcoin::Tx();
	/* Random onchain tx.  */
	tx = test_tx( "02000000000101385e378e72018e7e902980145fdbbfd16e6b0526bb63c656d5c1af3e63a6292e000000001716001471b2a7fc4fd6d6f4ee0d80db24a7bee788ba6442feffffff02636a9902000000001976a9143b9cdf5afe5640861cb96cdb3af5eca4216e303388ac431663720000000017a91471c7ff853df638d8bc076ba0841dcc30749ed74f870247304402204e4c50c34727caf658ecf0455d67d320078a5861e7bbe1fd207bbd720a40f7bd0220234067858406eb86cec3b0ff1d10612dc2bfc1a1d63782aeb0d9aea688d2c7f20121027ee675512d19023e72ba3c75ba5738e0e56979d6916da9bba8ddf4cc331942d0eae80900"
		    , "31e394b1c76d6162b95487e6719a0d4a4b585a0a22c96980978a5c6ee2c06197"
		    );
	/* Details below were extracted from blockstream.info.  */
	assert(tx.nVersion == 2);
	assert(tx.inputs.size() == 1);
	assert(tx.inputs[0].prevTxid == Bitcoin::TxId("2e29a6633eafc1d556c663bb26056b6ed1bfdb5f148029907e8e01728e375e38"));
	assert(tx.inputs[0].prevOut == 0);
	assert(tx.inputs[0].scriptSig == Util::Str::hexread("16001471b2a7fc4fd6d6f4ee0d80db24a7bee788ba6442"));
	assert(tx.inputs[0].witness.witnesses.size() == 2);
	assert(tx.inputs[0].witness.witnesses[0] == Util::Str::hexread("304402204e4c50c34727caf658ecf0455d67d320078a5861e7bbe1fd207bbd720a40f7bd0220234067858406eb86cec3b0ff1d10612dc2bfc1a1d63782aeb0d9aea688d2c7f201"));
	assert(tx.inputs[0].witness.witnesses[1] == Util::Str::hexread("027ee675512d19023e72ba3c75ba5738e0e56979d6916da9bba8ddf4cc331942d0"));
	assert(tx.inputs[0].nSequence == 0xfffffffe);
	assert(tx.outputs.size() == 2);
	assert(tx.outputs[0].amount == Ln::Amount::sat(43608675));
	assert(tx.outputs[0].scriptPubKey == Util::Str::hexread("76a9143b9cdf5afe5640861cb96cdb3af5eca4216e303388ac"));
	assert(tx.outputs[1].amount == Ln::Amount::sat(1919096387));
	assert(tx.outputs[1].scriptPubKey == Util::Str::hexread("a91471c7ff853df638d8bc076ba0841dcc30749ed74f87"));
	assert(tx.nLockTime == 649450);

	return 0;
}
