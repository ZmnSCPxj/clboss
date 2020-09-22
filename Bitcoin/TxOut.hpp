#ifndef BITCOIN_TXOUT_HPP
#define BITCOIN_TXOUT_HPP

#include<cstdint>
#include<iostream>
#include<vector>
#include"Ln/Amount.hpp"

namespace Bitcoin {

/** struct Bitcoin::TxOut
 *
 * @brief represents an output of a Bitcoin
 * transaction.
 *
 * @desc This includes an amount plus a
 * script.
 */
struct TxOut {
	Ln::Amount amount;
	std::vector<std::uint8_t> scriptPubKey;

	bool operator==(TxOut const& o) const {
		return amount == o.amount
		    && scriptPubKey == o.scriptPubKey
		     ;
	}
	bool operator!=(TxOut const& o) const {
		return !(*this == o);
	}
};

}

std::ostream& operator<<(std::ostream&, Bitcoin::TxOut const&);
std::istream& operator>>(std::istream&, Bitcoin::TxOut&);

#endif /* !defined(BITCOIN_TXOUT_HPP) */
