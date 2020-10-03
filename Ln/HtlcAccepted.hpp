#ifndef LN_HTLCACCEPTED_HPP
#define LN_HTLCACCEPTED_HPP

#include"Ln/Amount.hpp"
#include"Ln/Preimage.hpp"
#include"Ln/Scid.hpp"
#include"Sha256/Hash.hpp"
#include<cstdint>
#include<memory>
#include<vector>

namespace Ln { namespace HtlcAccepted {

/** struct Ln::HtlcAccepted::Request
 *
 * @brief Represents the payload of an `htlc_accepted`
 * hook.
 */
struct Request {
	/* Command id.  */
	std::uint64_t id;

	/* Incoming data.  */
	/* Raw data.  */
	std::vector<std::uint8_t> incoming_payload;
	/* Incoming amount.  */
	Ln::Amount incoming_amount;
	/* Incoming CLTV.  */
	std::uint32_t incoming_cltv;

	/* Outgoing details.  */
	/* True if type=tlv.  */
	bool type_tlv;
	/* Next channel.
	 * `nullptr` if we are the recipient.
	 */
	Ln::Scid next_channel;
	/* Outgoing amount.
	 * Equal to `incoming_amount` if we are the recepient.
	 */
	Ln::Amount next_amount;
	/* Outgoing CLTV.  Invalid if we are recepient.  */
	std::uint32_t next_cltv;
	/* Outgoing onion.  */
	std::vector<std::uint8_t> next_onion;

	/* Information.  */
	/* Payment hash to claim with.  */
	Sha256::Hash payment_hash;
	/* Verification that previous hop is not trying to probe.
	 * 0x00000.. if not present.
	 */
	Ln::Preimage payment_secret;
};

/** class Ln::HtlcAccepted::Response
 *
 * @brief opaque type that may be any of
 * three different things.
 */
class Response {
private:
	class Impl;
	std::shared_ptr<Impl> pimpl;

public:
	/* std::shared_ptr is so convenient... */
	Response() =default;
	Response(Response&&) =default;
	Response(Response const&) =default;
	Response& operator=(Response&&) =default;
	Response& operator=(Response const&) =default;
	~Response() =default;

	/* If default-constructed or moved from,
	 * returns false.
	 */
	operator bool() const;
	bool operator!() const {
		return !bool(*this);
	}

	/* Factories.  */
	static Response cont(std::uint64_t id);
	static Response fail( std::uint64_t id
			    , std::vector<std::uint8_t> message
			    );
	static Response resolve( std::uint64_t id
			       , Ln::Preimage preimage
			       );

	/* Queries.  */
	bool is_cont() const;
	bool is_fail() const;
	bool is_resolve() const;

	/* Data extraction.  */
	std::uint64_t id() const;
	std::vector<std::uint8_t> const& fail_message() const;
	Ln::Preimage const& resolve_preimage() const;
};

}}

#endif /* !defined(LN_HTLCACCEPTED_HPP) */
