#ifndef BITCOIN_TXIN_HPP
#define BITCOIN_TXIN_HPP

#include<cstdint>
#include<iostream>
#include<vector>
#include"Bitcoin/TxId.hpp"
#include"Bitcoin/WitnessField.hpp"

namespace Bitcoin {

/** struct Bitcoin::TxIn
 *
 * @brief represents a Bitcoin transaction input.
 *
 * @desc note this does ***not*** include the
 * corresponding witness stack for this input.
 *
 * An input contains these information:
 *
 * - Previous transaction ID.
 *   This is serialized in reverse order compared
 *   to how it is usually seen in explorer sites
 *   and used in most interfaces.
 * - Previous transaction output being spent.
 * - `scriptSig`.
 * - `nSequence`.
 * - The `witness` for the input.
 *   Note that the serialization of a `TxIn` does
 *   ***not*** include the `witness`, it must be
 *   (de)serialized separately.
 */
struct TxIn {
	Bitcoin::TxId prevTxid;
	std::uint32_t prevOut;
	std::vector<std::uint8_t> scriptSig;
	std::uint32_t nSequence;
	Bitcoin::WitnessField witness;

	TxIn() : prevOut(0xFFFFFFFF), nSequence(0xFFFFFFFF) { }
	bool operator==(TxIn const& o) const {
		return prevTxid == o.prevTxid
		    && prevOut == o.prevOut
		    && scriptSig == o.scriptSig
		    && nSequence == o.nSequence
		    && witness == o.witness
		     ;
	}
	bool operator!=(TxIn const& o) const {
		return !(*this == o);
	}
};

}

std::ostream& operator<<(std::ostream&, Bitcoin::TxIn const&);
std::istream& operator>>(std::istream&, Bitcoin::TxIn&);

#endif /* !defined(BITCOIN_TXIN_HPP) */
