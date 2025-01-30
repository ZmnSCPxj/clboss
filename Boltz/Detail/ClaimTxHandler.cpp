#include"Boltz/Detail/ClaimTxHandler.hpp"
#include"Boltz/Detail/compute_preimage.hpp"
#include"Boltz/Detail/find_lockup_outnum.hpp"
#include"Boltz/Detail/initial_claim_tx.hpp"
#include"Boltz/EnvIF.hpp"
#include"Ev/Io.hpp"
#include"Secp256k1/SignerIF.hpp"
#include"Sqlite3.hpp"
#include"Util/Str.hpp"
#include<sstream>

namespace {

/* Throw to early-out of the run.  */
struct End { };

}


namespace Boltz { namespace Detail {

std::string ClaimTxHandler::logprefix(std::string msg) {
	return std::string("Boltz::Service(\"") + api_endpoint + "\"): "
	     + " \"" + swapId + "\": "
	     + msg
	     ;
}
Ev::Io<void> ClaimTxHandler::logt(std::string msg) {
	return env.logt(logprefix(msg));
}
Ev::Io<void> ClaimTxHandler::logd(std::string msg) {
	return env.logd(logprefix(msg));
}
Ev::Io<void> ClaimTxHandler::loge(std::string msg) {
	return env.loge(logprefix(msg));
}

Ev::Io<void> ClaimTxHandler::run() {
	auto self = shared_from_this();
	/* Keep self alive until the run ends.  */
	return Ev::lift().then([self]() {
		return self->core_run();
	}).then([self]() {
		return Ev::lift();
	});
}

Ev::Io<void> ClaimTxHandler::core_run() {
	return Ev::lift().then([this]() {

		/* Fetch data from database.  */
		return db.transact();
	}).then([this](Sqlite3::Tx tx) {
		/* First do a quick check.  */
		auto check1 = tx.query(R"QRY(
		SELECT lockedUp, timeoutBlockheight, destinationAddress
		  FROM "BoltzServiceFactory_rsub"
		 WHERE apiAccess = :apiAccess
		   AND swapId = :swapId
		     ;
		)QRY")
			.bind(":apiAccess", api_endpoint)
			.bind(":swapId", swapId)
			.execute();
		auto found = 0;
		auto lockedUp = false;
		for (auto& r : check1) {
			++found;
			lockedUp = r.get<bool>(0);
			timeoutBlockheight = r.get<std::uint32_t>(1);
			destinationAddress = r.get<std::string>(2);
			break;
		}
		if (found == 0) {
			/* Unexpected not found!  */
			return loge("Swap ID not in table.").then([]() {
				throw End();
				return Ev::lift();
			});
		}
		if (found > 1) {
			/* Unexpected duplicate!  */
			return loge("Swap ID duplicated??").then([]() {
				throw End();
				return Ev::lift();
			});
		}
		if (lockedUp) {
			/* TODO: Bump up instead of doing nothing.  */
			return logd( "Already broadcasted claim tx."
				   ).then([]() {
				throw End();
				return Ev::lift();
			});
		}
		if (destinationAddress.empty()) {
			return loge("Swap destination address is empty??"
				   ).then([]() {
				throw End();
				return Ev::lift();
			});
		}
		// TODO: Check for validity of destination address?

		/* Perform the actual fetch of the data.  */
		auto fetch = tx.query(R"QRY(
		SELECT tweak -- 0
		     , preimage -- 1
		     , destinationAddress -- 2
		     , redeemScript -- 3
		     , timeoutBlockheight -- 4
		     , onchainAmount -- 5
		  FROM "BoltzServiceFactory_rsub"
		 WHERE apiAccess = :apiAccess
		   AND swapId = :swapId
		     ;
		)QRY")
			.bind(":apiAccess", api_endpoint)
			.bind(":swapId", swapId)
			.execute();
		for (auto& r : fetch) {
			tweak = Secp256k1::PrivKey(r.get<std::string>(0));
			preimage = Ln::Preimage(r.get<std::string>(1));
			destinationAddress = r.get<std::string>(2);
			redeemScript = Util::Str::hexread(
				r.get<std::string>(3)
			);
			timeoutBlockheight = r.get<std::uint32_t>(4);
			onchainAmount = Ln::Amount::sat(
				r.get<std::uint64_t>(5)
			);
		}
		tx.commit();

		/* Is the timeout too near?  */
		if (timeoutBlockheight <= blockheight + 10) {
			auto os = std::ostringstream();
			os << "Timeout " << timeoutBlockheight
			   << " too near to current time "
			   << blockheight
			    ;
			return logd(os.str()).then([]() {
				throw End();
				return Ev::lift();
			});
		}

		/* Find the outnum from the lockup_tx.  */
		auto outnum = Detail::find_lockup_outnum( lockup_tx
							, redeemScript
							);
		if (outnum < 0) {
			auto msg = std::string("Service lockup tx ")
				 + std::string(lockup_tx) + " "
				 + "does not pay to lockscript."
				 ;
			return loge(msg).then([]() {
				throw End();
				return Ev::lift();
			});
		}
		lockupOut = std::size_t(outnum);

		/* Check the onchain amount is correct.  */
		if ( lockup_tx.outputs[lockupOut].amount
		  != onchainAmount
		   ) {
			auto msg = std::string("Service lockup tx ")
				 + std::string(lockup_tx) + " "
				 + "does not pay expected amount."
				 ;
			return loge(msg).then([]() {
				throw End();
				return Ev::lift();
			});
		}

		/* Now compute the real preimage from the signer key
		 * and the in-database preimage.  */
		real_preimage = Detail::compute_preimage(signer, preimage);

		return Ev::lift();
	}).then([this]() {

		/* Now look up the feerate.  */
		return env.get_feerate();
	}).then([this](std::uint32_t feerate) {
		/* Generate the claim tx.  */
		Detail::initial_claim_tx( claim_tx
					, lockupClaimFees

					, feerate
					, blockheight
					, lockup_txid
					, lockupOut
					, onchainAmount

					, signer
					, tweak
					, real_preimage
					, redeemScript

					, destinationAddress
					);
		/* Log it.  */
		auto msg = std::string("Broadcasting claim tx: ")
			 + std::string(claim_tx)
			 ;
		return logd(msg);
	}).then([this]() {

		/* Do the broadcast.  */
		return env.broadcast_tx(claim_tx);
	}).then([this](bool success) {
		if (!success) {
			auto msg = std::string("Broadcast failed! ")
				 + std::string(claim_tx)
				 ;
			return loge(msg).then([]() {
				throw End();
				return Ev::lift();
			});
		}
		return Ev::lift();
	}).then([this]() {

		/* Update the db.  */
		return db.transact();
	}).then([this](Sqlite3::Tx tx) {
		tx.query(R"QRY(
		UPDATE "BoltzServiceFactory_rsub"
		   SET lockedUp = 1
		     , lockupTxid = :lockup_txid
		     , lockupOut = :lockupOut
		     , lockupConfirmedHeight = :blockheight
		     , lockupClaimFees = :lockupClaimFees
		     ;
		)QRY")
			.bind(":lockup_txid", std::string(lockup_txid))
			.bind(":lockupOut", lockupOut)
			.bind(":blockheight", blockheight)
			.bind(":lockupClaimFees", lockupClaimFees.to_sat())
			.execute();
		tx.commit();

		return Ev::lift();
	}).catching<End>([](End const&) {
		/* Lambda, the ultimate Swiss Army Knife.... */
		return Ev::lift();
	});
}

}}
