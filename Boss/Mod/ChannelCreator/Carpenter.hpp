#ifndef BOSS_MOD_CHANNELCREATOR_CARPENTER_HPP
#define BOSS_MOD_CHANNELCREATOR_CARPENTER_HPP

#include<map>
#include<queue>

namespace Boss { namespace Mod { class Rpc; }}
namespace Boss { namespace Mod { class Waiter; }}
namespace Ev { template<typename a> class Io; }
namespace Json { class Out; }
namespace Ln { class Amount; }
namespace Ln { class NodeId; }
namespace S { class Bus; }

namespace Boss { namespace Mod { namespace ChannelCreator {

/** class Boss::Mod::ChannelCreator::Carpenter
 *
 * @brief Actual module that builds channels
 * according to a plan given to it.
 */
class Carpenter {
private:
	S::Bus& bus;
	Boss::Mod::Waiter& waiter;
	Boss::Mod::Rpc* rpc;

	void start();

	Json::Out
	json_plan(std::map<Ln::NodeId, Ln::Amount> const& plan);
	Json::Out
	json_single_plan(Ln::NodeId const&, Ln::Amount const&);

	Ev::Io<void>
	connect_1(Ln::NodeId node);
	Ev::Io<void>
	report_channelings(std::queue<Ln::NodeId> nodes, bool);

public:
	Carpenter() =delete;
	Carpenter(Carpenter const&) =delete;
	Carpenter(Carpenter&&) =delete;

	explicit
	Carpenter( S::Bus& bus_
		 , Boss::Mod::Waiter& waiter_
		 ) : bus(bus_)
		   , waiter(waiter_)
		   , rpc(nullptr)
		   { start(); }

	/** Boss::Mod::ChannelCreator::Carpenter::construct
	 *
	 * @brief have the carpenter construct channels,
	 * assigning specific values to specific nodes.
	 */
	Ev::Io<void>
	construct(std::map<Ln::NodeId, Ln::Amount> plan);
};

}}}

#endif /* !defined(BOSS_MOD_CHANNELCREATOR_CARPENTER_HPP) */
