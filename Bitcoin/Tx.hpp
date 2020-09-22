#ifndef BITCOIN_TX_HPP
#define BITCOIN_TX_HPP

#include"Bitcoin/TxIn.hpp"
#include"Bitcoin/TxOut.hpp"

namespace Bitcoin { class TxId; }

namespace Bitcoin {

/** struct Bitcoin::Tx
 *
 * @brief represents a complete Bitcoin
 * transaction.
 */
struct Tx {
	std::uint32_t nVersion;
	std::vector<TxIn> inputs;
	std::vector<TxOut> outputs;
	std::uint32_t nLockTime;

	Bitcoin::TxId get_txid() const;

	Tx() : nVersion(2), nLockTime(0) { }

	explicit
	Tx(std::string const&);
};

}

std::ostream& operator<<(std::ostream&, Bitcoin::Tx const&);
std::istream& operator>>(std::istream&, Bitcoin::Tx&);

#endif /* !defined(BITCOIN_TX_HPP) */
