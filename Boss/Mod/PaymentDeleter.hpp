#ifndef BOSS_MOD_PAYMENTDELETER_HPP
#define BOSS_MOD_PAYMENTDELETER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::PaymentDeleter
 *
 * @brief Deletes outgoing payments that CLBOSS initiated.
 */
class PaymentDeleter {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	PaymentDeleter() =delete;

	~PaymentDeleter();
	PaymentDeleter(PaymentDeleter&&);

	explicit
	PaymentDeleter(S::Bus& bus);
};

}}

#endif /* !defined(BOSS_MOD_PAYMENTDELETER_HPP) */
