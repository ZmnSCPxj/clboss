#ifndef BITCOIN_TXID_HPP
#define BITCOIN_TXID_HPP

#include"Sha256/Hash.hpp"
#include<iostream>

namespace Bitcoin {

/** struct Bitcoin::TxId
 *
 * @brief the reverse double-sha256 of the non-witness
 * serialization of a transaction.
 *
 * @desc Transactions IDs are just the double-SHA256
 * of a particular encoding of a transaction without
 * the witness part of transactions, but in reverse
 * order, because reasons.
 */
class TxId;
}

/* NOTE: These output in binary.  */
std::ostream& operator<<(std::ostream&, Bitcoin::TxId const&);
std::istream& operator>>(std::istream&, Bitcoin::TxId&);

namespace Bitcoin {

class TxId {
private:
	/* The hash stored here is already in reverse order.  */
	Sha256::Hash hash;

	friend
	std::ostream& ::operator<<(std::ostream&, Bitcoin::TxId const&);
	friend
	std::istream& ::operator>>(std::istream&, Bitcoin::TxId&);
public:
	TxId() =default;
	TxId(TxId const&) =default;
	TxId(TxId&&) =default;
	TxId& operator=(TxId const&) =default;
	TxId& operator=(TxId&&) =default;
	~TxId() =default;

	/* Must be expressed in the expected reversed
	 * order.
	 */
	explicit
	TxId(std::string const& s);
	/* Performs the reversal of bytes.  */
	explicit
	TxId(Sha256::Hash hash_);
	/* Prints out in the reversed order.  */
	explicit
	operator std::string() const;

	bool operator==(Bitcoin::TxId const& o) const {
		return hash == o.hash;
	}
	bool operator!=(Bitcoin::TxId const& o) const {
		return hash != o.hash;
	}
};

}

#endif /* !defined(BITCOIN_TXID_HPP) */
