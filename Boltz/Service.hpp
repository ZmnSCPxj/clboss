#ifndef BOLTZ_SERVICE_HPP
#define BOLTZ_SERVICE_HPP

#include<cstdint>
#include<string>

namespace Ev { template<typename a> class Io; }
namespace Ln { class Amount; }

namespace Boltz {

/** class Boltz::Service
 *
 * @brief represents a particular BOLTZ
 * instance.
 *
 * @desc we are only interested in the
 * "reverse submarine" service, i.e.
 * offchain-to-onchain swap.
 */
class Service {
public:
	virtual ~Service() {}

	/** Boltz::Service::on_block
	 *
	 * @brief call this at every block in order
	 * to ensure that swaps complete correctly.
	 */
	virtual
	Ev::Io<void> on_block(std::uint32_t) =0;

	/** Boltz::Service::get_quotation
	 *
	 * @brief return the fees it would take
	 * to get a particular amount onchain.
	 *
	 * @desc this only considers amount from
	 * the BOLTZ instance, and not to claim
	 * it and put it into unilateral control.
	 */
	virtual
	Ev::Io<Ln::Amount>
	get_quotation(Ln::Amount) =0;

	/** Boltz::Service::swap
	 *
	 * @brief create a reverse submarine swap
	 * paying out to the given address.
	 *
	 * @return the invoice to pay.
	 *
	 * @desc if paying the invoice fails, the
	 * swap should be considered as automatically
	 * cancelled.
	 * The invoice has an expiry, after which it
	 * must no longer be paid.
	 */
	virtual
	Ev::Io<std::string>
	swap( Ln::Amount
	    , std::string onchain_address
	    ) =0;
};

}

#endif /* !defined(BOLTZ_SERVICE_HPP) */
