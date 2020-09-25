#include"Bitcoin/Tx.hpp"
#include"Boltz/Detail/find_lockup_outnum.hpp"
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"
#include<algorithm>

namespace Boltz { namespace Detail {

int find_lockup_outnum( Bitcoin::Tx const& tx
		      , std::vector<std::uint8_t> const& redeemScript
		      ) {
	auto hash = Sha256::fun(
		&redeemScript[0], redeemScript.size()
	);
	auto scriptPubKey = std::vector<std::uint8_t>(34);
	scriptPubKey[0] = 0x00;
	scriptPubKey[1] = 0x20;
	hash.to_buffer(&scriptPubKey[2]);

	auto it = std::find_if( tx.outputs.begin(), tx.outputs.end()
			      , [&scriptPubKey](Bitcoin::TxOut const& out) {
		return out.scriptPubKey == scriptPubKey;
	});
	if (it == tx.outputs.end())
		return -1;
	return it - tx.outputs.begin();
}

}}
