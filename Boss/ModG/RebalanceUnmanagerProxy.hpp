#ifndef BOSS_MODG_REBALANCEUNMANAGERPROXY_HPP
#define BOSS_MODG_REBALANCEUNMANAGERPROXY_HPP

#include"Boss/ModG/ReqResp.hpp"
#include"Boss/Msg/RequestRebalanceUnmanaged.hpp"
#include"Boss/Msg/ResponseRebalanceUnmanaged.hpp"
#include"Ln/NodeId.hpp"

namespace Boss { namespace ModG {

/** class Boss::ModG::RebalanceUnmanagerProxy
 *
 * @brief a proxy for Boss::Mod::RebalanceUnmanager,
 * allowing to query the "balance"-unmanaged nodes.
 */
class RebalanceUnmanagerProxy {
private:
	ReqResp< Msg::RequestRebalanceUnmanaged
	       , Msg::ResponseRebalanceUnmanaged
	       > core;

public:
	RebalanceUnmanagerProxy() =delete;
	RebalanceUnmanagerProxy(RebalanceUnmanagerProxy const&) =delete;
	RebalanceUnmanagerProxy(RebalanceUnmanagerProxy&&) =delete;

	explicit
	RebalanceUnmanagerProxy(S::Bus& bus)
		: core( bus
		      , [](Msg::RequestRebalanceUnmanaged& m, void* p) {
				m.requester = p;
		        }
		      , [](Msg::ResponseRebalanceUnmanaged& m) {
				return m.requester;
		        }
		      )
		{ }

	/* Returns a pointer since `Ev::Io` expects a movable object,
	 * and references are not quite movable.  */
	Ev::Io<std::set<Ln::NodeId> const*>
	get_unmanaged() {
		return core.execute(Msg::RequestRebalanceUnmanaged{
			nullptr
		}).then([](Msg::ResponseRebalanceUnmanaged m) {
			return Ev::lift(&m.unmanaged);
		});
	}
};

}}

#endif /* !defined(BOSS_MODG_REBALANCEUNMANAGERPROXY_HPP) */
