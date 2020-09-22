#ifndef BITCOIN_WITNESSFIELD_HPP
#define BITCOIN_WITNESSFIELD_HPP

#include<cstdint>
#include<iostream>
#include<vector>

namespace Bitcoin {

/** struct Bitcoin::WitnessField
 *
 * @brief represents the witness field of a
 * particular `TxIn`.
 *
 * @desc Witness fields can be empty.
 * If all inputs of a transaction have empty
 * witness fields, then the serialization of
 * that transaction uses the pre-SegWit
 * serialization.
 * If at least one input has a non-empty
 * witness field, then the transaction
 * serialization uses the `wtxid`
 * serialization.
 *
 * The witness field can represent a stack
 * of witness items if the input being spent
 * is a P2WSH.
 * Index [0] is the stack bottom and the last
 * item is the stack top.
 * For a P2WSH the script to be executed is
 * the stack top, and its input is the rest
 * of the stack.
 */
struct WitnessField {
	std::vector<std::vector<std::uint8_t>> witnesses;

	bool empty() const {
		return witnesses.empty();
	}
	bool operator==(WitnessField const& o) const {
		return witnesses == o.witnesses;
	}
	bool operator!=(WitnessField const& o) const {
		return !(*this == o);
	}
};

}

std::ostream& operator<<(std::ostream&, Bitcoin::WitnessField const&);
std::istream& operator>>(std::istream&, Bitcoin::WitnessField&);

#endif /* !defined(BITCOIN_WITNESSFIELD_HPP) */
