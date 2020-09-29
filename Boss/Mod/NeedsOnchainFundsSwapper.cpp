#include"Boss/Mod/NeedsOnchainFundsSwapper.hpp"
#include"Boss/Msg/Block.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/NeedsOnchainFunds.hpp"
#include"Boss/Msg/OnchainFee.hpp"
#include"Boss/Msg/SwapRequest.hpp"
#include"Boss/Msg/SwapResponse.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Ln/Amount.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Util/make_unique.hpp"
#include"Uuid.hpp"

namespace {

/* Minimum amount we are willing to swap.  */
auto const min_amount = Ln::Amount::btc(0.0009);
/* Number of blocks we ignore NedsOnchainFunds.  */
auto const time_between = std::uint32_t(12);
/* Number of blocks that we wait for a swap, and force-forget the swap.  */
auto const swap_timeout = std::uint32_t(432);

/* Query to initialize our tables.  */
auto const dbinit = R"QRY(
CREATE TABLE IF NOT EXISTS
       "NeedsOnchainFundsSwapper"
     ( id INTEGER PRIMARY KEY
     -- if empty string, we are not waiting
     -- on a swap to complete.
     -- if non-empty string, a swap is
     -- ongoing and we should ignore
     -- NeedsOnchainFunds.
     , swapUuid TEXT NOT NULL
     -- if blockheight is below this,
     -- ignore NeedsOnchainFunds.
     , timeout INTEGER NOT NULL
     );
INSERT OR IGNORE
  INTO "NeedsOnchainFundsSwapper"
VALUES( 0
      , ''
      , 0
      );

-- ALTER TABLE does not have an "OR IGNORE" variant,
-- so we have to switch tables...

CREATE TABLE IF NOT EXISTS
       "NeedsOnchainFundsSwapperv2"
     ( id INTEGER PRIMARY KEY
     -- if empty string, we are not waiting
     -- on a swap to complete.
     -- if non-empty string, a swap is
     -- ongoing and we should ignore
     -- NeedsOnchainFunds.
     , swapUuid TEXT NOT NULL
     -- if blockheight is below this,
     -- ignore NeedsOnchainFunds.
     , timeout INTEGER NOT NULL
     -- If we reach this blockheight with
     -- the swap not being resolved, force
     -- ignoring the swap result.
     , absTimeout INTEGER NOT NULL
     );
INSERT OR IGNORE
  INTO "NeedsOnchainFundsSwapperv2"
SELECT id, swapUuid, timeout, timeout+432
  FROM "NeedsOnchainFundsSwapper"
 WHERE id = 0
     ;
DROP TABLE "NeedsOnchainFundsSwapper";
)QRY";

}

namespace Boss { namespace Mod {

class NeedsOnchainFundsSwapper::Impl {
private:
	S::Bus& bus;

	Sqlite3::Db db;
	std::unique_ptr<bool> low_fee;
	std::unique_ptr<std::uint32_t> blockheight;

public:
	Impl() =delete;
	Impl(S::Bus& bus_) : bus(bus_) { start(); }

private:
	void start() {
		bus.subscribe<Msg::DbResource
			     >([this](Msg::DbResource const& r) {
			db = r.db;
			return db.transact().then([this](Sqlite3::Tx tx) {
				tx.query_execute(dbinit);
				tx.commit();
				return Ev::lift();
			});
		});
		bus.subscribe<Msg::Block
			     >([this](Msg::Block const& block) {
			if (!blockheight)
				blockheight = Util::make_unique<std::uint32_t
							       >();
			*blockheight = block.height;
			if (db)
				return on_block();
			return Ev::lift();
		});
		bus.subscribe<Msg::OnchainFee
			     >([this](Msg::OnchainFee const& m) {
			if (!low_fee)
				low_fee = Util::make_unique<bool>();
			*low_fee = m.fees_low;
			return Ev::lift();
		});

		bus.subscribe<Msg::NeedsOnchainFunds
			     >([this](Msg::NeedsOnchainFunds const& m) {
			/* Ignore if we have incomplete information.  */
			if (!db || !blockheight || !low_fee)
				return Ev::lift();
			return on_needs_onchain_funds(m.needed);
		});
		bus.subscribe<Msg::SwapResponse
			     >([this](Msg::SwapResponse const& r) {
			return on_swap_response(r);
		});
	}

	Ev::Io<void> on_needs_onchain_funds(Ln::Amount needed) {
		auto amount = needed * 1.12;
		if (amount < min_amount)
			amount = min_amount;
		/* Check database.  */
		return db.transact().then([this, amount](Sqlite3::Tx tx) {
			/* Extract the relevant data.  */
			bool has_swap = false;
			auto timeout = std::uint32_t();
			auto fetch = tx.query(R"QRY(
			SELECT swapUuid, timeout
			  FROM "NeedsOnchainFundsSwapperv2"
			 WHERE id = 0
			     ;
			)QRY")
				.execute();
			for (auto& r : fetch) {
				has_swap = r.get<std::string>(0)
					!= "";
				timeout = r.get<std::uint32_t>(1);
			}
			if (has_swap || *blockheight <= timeout) {
				/* Do nothing.  */
				tx.commit();
				return Ev::lift();
			}
			if (!*low_fee) {
				/* Set timeout to a later point,
				 * then log and stop.  */
				timeout = *blockheight + time_between;
				tx.query(R"QRY(
				UPDATE "NeedsOnchainFundsSwapperv2"
				   SET timeout = :timeout
				 WHERE id = 0
				     ;
				)QRY")
					.bind(":timeout", timeout)
					.execute();
				tx.commit();
				return Boss::log( bus, Info
						, "NeedsOnchainFundsSwapper: "
						  "Fees are high, will wait."
						);
			}

			/* Now we have to do a swap.  */
			auto swapUuid = Uuid::random();
			timeout = *blockheight + time_between;
			auto absTimeout = *blockheight + swap_timeout;
			tx.query(R"QRY(
			UPDATE "NeedsOnchainFundsSwapperv2"
			   SET swapUuid = :swapUuid
			     , timeout = :timeout
			     , absTimeout = :absTimeout
			 WHERE id = 0
			     ;
			)QRY")
				.bind(":swapUuid"
				     , std::string(swapUuid)
				     )
				.bind(":timeout", timeout)
				.bind(":absTimeout", absTimeout)
				.execute();

			/* Prepare the request message.  */
			auto dbtx = std::make_shared<Sqlite3::Tx>(
				std::move(tx)
			);
			auto msg = std::make_shared<Msg::SwapRequest>();
			msg->dbtx = dbtx;
			msg->id = swapUuid;
			msg->min_offchain_amount = min_amount;
			msg->max_offchain_amount = amount;

			/* Log, then send request.  */
			return Boss::log( bus, Info
					, "NeedsOnchainFundsSwapper: "
					  "Starting swap %s for %s to get "
					  "onchain funds."
					).then([this, msg]() {
				return bus.raise(std::move(*msg));
			});
		});
	}

	Ev::Io<void>
	on_swap_response(Msg::SwapResponse const& r) {
		/* Borrow, do not acquire --- if this swap is not for us,
		 * we should not commit the transaction!  */
		auto& tx = *r.dbtx;
		/* There is an edge case where we receive this message
		 * but there is no db yet, which might mean that we do
		 * not have a table yet.
		 * Fortunately the table initialization is idempotent,
		 * so we can safely just do the queries here.
		 */
		tx.query_execute(dbinit);
		/* See if it matches.  */
		auto fetch = tx.query(R"QRY(
		SELECT timeout
		  FROM "NeedsOnchainFundsSwapperv2"
		 WHERE id = 0
		   AND swapUuid == :swapUuid
		     ;
		)QRY")
			.bind(":swapUuid", std::string(r.id))
			.execute();
		auto found = false;
		auto timeout = std::uint32_t();
		for (auto& r : fetch) {
			found = true;
			timeout = r.get<std::uint32_t>(0);
		}
		/* If not matched, ignore.  */
		if (!found)
			return Ev::lift();

		/* Matched, so we should clear our db.  */

		/* If we know the blockheight, add the margin
		 * to it to get the timeout, otherwise adjust
		 * the in-db timeout.  */
		timeout += time_between;
		if (blockheight) {
			auto propose = *blockheight + time_between;
			if (propose > timeout)
				timeout = propose;
		}

		tx.query(R"QRY(
		UPDATE "NeedsOnchainFundsSwapperv2"
		   SET swapUuid = ''
		     , timeout = :timeout
		     , absTimeout = 5000000
		 WHERE id = 0
		     ;
		)QRY")
			.bind(":timeout", timeout)
			.execute();

		/* Let swap manager commit.  */
		return Boss::log( bus, Info
				, "NeedsOnchainFundsSwapper: Done."
				);
	}

	Ev::Io<void> on_block() {
		return db.transact().then([this](Sqlite3::Tx tx) {
			/* Check if we reached the absolute timeout.  */
			auto absTimeout = std::uint32_t();
			auto fetch = tx.query(R"QRY(
			SELECT absTimeout
			  FROM "NeedsOnchainFundsSwapperv2"
			 WHERE id = 0
			     ;
			)QRY")
				.execute();
			for (auto& r : fetch)
				absTimeout = r.get<std::uint32_t>(0);
			/* Not reached yet, ignore.  */
			if (*blockheight < absTimeout)
				return Ev::lift();

			/* Forget the current swapUuid.  */
			tx.query_execute(R"QRY(
			UPDATE "NeedsOnchainFundsSwapperv2"
			   SET swapUuid = ''
			     , absTimeout = 5000000
			 WHERE id = 0
			     ;
			)QRY");
			tx.commit();

			return Boss::log( bus, Warn
					, "NeedsOnchainFundsSwapper: "
					  "Forgetting current swap, it is "
					  "too long to resolve..."
					);
		});
	}
};


NeedsOnchainFundsSwapper::NeedsOnchainFundsSwapper(NeedsOnchainFundsSwapper&&)
	=default;
NeedsOnchainFundsSwapper&
NeedsOnchainFundsSwapper::operator=(NeedsOnchainFundsSwapper&&) =default;
NeedsOnchainFundsSwapper::~NeedsOnchainFundsSwapper() =default;


NeedsOnchainFundsSwapper::NeedsOnchainFundsSwapper(S::Bus& bus)
	: pimpl(Util::make_unique<Impl>(bus)) { }

}}
