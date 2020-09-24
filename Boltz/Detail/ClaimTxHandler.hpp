#ifndef BOLTZ_DETAIL_CLAIMTXHANDLER_HPP
#define BOLTZ_DETAIL_CLAIMTXHANDLER_HPP

#include"Bitcoin/Tx.hpp"
#include"Bitcoin/TxId.hpp"
#include"Ln/Amount.hpp"
#include"Ln/Preimage.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Sqlite3/Db.hpp"
#include<cstdint>
#include<functional>
#include<memory>
#include<string>
#include<vector>

namespace Boltz { class EnvIF; }
namespace Ev { template<typename a> class Io; }
namespace Secp256k1 { class SignerIF; }

namespace Boltz { namespace Detail {

/** class Boltz::Detail::ClaimTxHandler
 *
 * @brief handles the claim transaction.
 *
 * @desc this is constructed and called at
 * each block for every swapID that is in
 * state "transaction.confirmed".
 */
class ClaimTxHandler : public std::enable_shared_from_this<ClaimTxHandler> {
private:
	Secp256k1::SignerIF& signer;
	Sqlite3::Db db;
	Boltz::EnvIF& env;
	std::string api_endpoint;
	std::string swapId;
	std::uint32_t blockheight;

	Bitcoin::Tx lockup_tx;
	Bitcoin::TxId lockup_txid;

	ClaimTxHandler( Secp256k1::SignerIF& signer_
		      , Sqlite3::Db db_
		      , Boltz::EnvIF& env_
		      , std::string const& api_endpoint_
		      , std::string const& swapId_
		      , std::uint32_t blockheight_
		      , Bitcoin::Tx lockup_tx_
		      ) : signer(signer_)
			, db(std::move(db_))
			, env(env_)
			, api_endpoint(api_endpoint_)
			, swapId(swapId_)
			, blockheight(blockheight_)
			, lockup_tx(std::move(lockup_tx_))
			{
		lockup_txid = lockup_tx.get_txid();
	}

public:
	static
	std::shared_ptr<ClaimTxHandler>
	create( Secp256k1::SignerIF& signer
	      , Sqlite3::Db db
	      , Boltz::EnvIF& env
	      , std::string const& api_endpoint
	      , std::string const& swapId
	      , std::uint32_t blockheight
	      , Bitcoin::Tx lockup_tx
	      ) {
		return std::shared_ptr<ClaimTxHandler>(
			new ClaimTxHandler( signer
					  , std::move(db)
					  , env
					  , api_endpoint
					  , swapId
					  , blockheight
					  , std::move(lockup_tx)
					  )
		);
	}

	Ev::Io<void> run();
private:
	Ev::Io<void> core_run();

	/* Wrappers.  */
	Ev::Io<void> logd(std::string);
	Ev::Io<void> loge(std::string);
	std::string logprefix(std::string);

	/* Additional data from table.  */
	Secp256k1::PrivKey tweak;
	Ln::Preimage preimage;
	std::string destinationAddress;
	std::vector<std::uint8_t> redeemScript;
	std::uint32_t timeoutBlockheight;
	Ln::Amount onchainAmount;

	/* Data we will store into the table.  */
	std::size_t lockupOut;
	Ln::Amount lockupClaimFees;

	/* Actual claim transaction.  */
	Bitcoin::Tx claim_tx;
};

}}

#endif /* !defined(BOLTZ_DETAIL_CLAIMTXHANDLER_HPP) */
