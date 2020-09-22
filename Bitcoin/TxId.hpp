#ifndef BITCOIN_TXID_HPP
#define BITCOIN_TXID_HPP

#include"Sha256/Hash.hpp"
#include<iostream>

namespace Bitcoin {

/** struct Bitcoin::TxId
 *
 * @brief represents a Bitcoin transaction ID.
 *
 * @desc Transactions IDs are just the double-SHA256
 * of a particular encoding of a transaction without
 * the witness part of transactions.
 */
class TxId;
}

/* NOTE: These output in binary.  */
std::ostream& operator<<(std::ostream&, Bitcoin::TxId const&);
std::istream& operator>>(std::istream&, Bitcoin::TxId&);

namespace Bitcoin {

class TxId {
private:
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

	explicit
	TxId(std::string const& s) : hash(s) { }
	explicit
	TxId(Sha256::Hash hash_) : hash(std::move(hash_)) { }
};

}

#endif /* !defined(BITCOIN_TXID_HPP) */
