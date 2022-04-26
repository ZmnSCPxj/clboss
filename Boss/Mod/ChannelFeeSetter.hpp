#ifndef BOSS_MOD_CHANNELFEESETTER_HPP
#define BOSS_MOD_CHANNELFEESETTER_HPP

#include"Boss/Msg/SetChannelFee.hpp"
#include"Ln/NodeId.hpp"
#include<queue>
#include<set>

namespace Boss { namespace Mod { class Rpc; }}
namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ChannelFeeSetter
 *
 * @brief Sets the channel fees according to
 * received `Boss::Msg::SetChannelFee` messages.
 */
class ChannelFeeSetter {
private:
	S::Bus& bus;
	Boss::Mod::Rpc* rpc;

	bool have_setchannel;

	std::vector<Boss::Msg::SetChannelFee> pending;

	void start();
	Ev::Io<void> set(Boss::Msg::SetChannelFee const& m);

	std::set<Ln::NodeId> unmanaged;

public:
	ChannelFeeSetter() =delete;
	ChannelFeeSetter(ChannelFeeSetter&&) =delete;
	ChannelFeeSetter(ChannelFeeSetter const&) =delete;

	explicit
	ChannelFeeSetter(S::Bus& bus_
			) : bus(bus_)
			  , rpc(nullptr)
			  , unmanaged()
			  { start(); }
};

}}

#endif /* !defined(BOSS_MOD_CHANNELFEESETTER_HPP) */
