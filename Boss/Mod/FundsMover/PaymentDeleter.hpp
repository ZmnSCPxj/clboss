#ifndef BOSS_MOD_FUNDSMOVER_PAYMENTDELETER_HPP
#define BOSS_MOD_FUNDSMOVER_PAYMENTDELETER_HPP

#include"Jsmn/Object.hpp"
#include<cstddef>
#include<string>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace FundsMover {

/** class Boss::Mod::FundsMover::PaymentDeleter
 *
 * @brief temporary runtime object to keep track of
 * payments, in order to delete.
 */
class PaymentDeleter {
private:
	S::Bus& bus;
	Boss::Mod::Rpc& rpc;

	Jsmn::Object pays;
	Jsmn::Object::iterator it;
	std::size_t count;

	explicit
	PaymentDeleter( S::Bus& bus_
		      , Boss::Mod::Rpc& rpc_
		      ) : bus(bus_), rpc(rpc_) { }

	Ev::Io<void> core_run();
	Ev::Io<void> loop();
	Ev::Io<void> delpay( std::string const& payment_hash
			   , std::string const& status
			   );

public:
	PaymentDeleter() =delete;
	PaymentDeleter(PaymentDeleter&&) =delete;
	PaymentDeleter(PaymentDeleter const&) =delete;

	static
	Ev::Io<void> run( S::Bus& bus
			, Boss::Mod::Rpc& rpc
			);
};

}}}

#endif /* !defined(BOSS_MOD_FUNDSMOVER_PAYMENTDELETER_HPP) */
