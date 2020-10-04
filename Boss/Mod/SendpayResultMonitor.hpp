#ifndef BOSS_MOD_SENDPAYRESULTMONITOR_HPP
#define BOSS_MOD_SENDPAYRESULTMONITOR_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::SendpayResultMonitor
 *
 * @brief monitors result of `sendpay`/`sendonion` commands,
 * and broadcasts `Boss::Msg::SendpayResult` messages.
 */
class SendpayResultMonitor {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	SendpayResultMonitor() =delete;

	SendpayResultMonitor(SendpayResultMonitor&&);
	~SendpayResultMonitor();

	explicit
	SendpayResultMonitor(S::Bus&);
};

}}

#endif /* !defined(BOSS_MOD_SENDPAYRESULTMONITOR_HPP) */
