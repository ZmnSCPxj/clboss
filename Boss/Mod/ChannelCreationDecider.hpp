#ifndef BOSS_MOD_CHANNELCREATIONDECIDER_HPP
#define BOSS_MOD_CHANNELCREATIONDECIDER_HPP

#include<memory>

namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ChannelCreationDecider
 *
 * @brief module that decides whether to create
 * channels whenever `Boss::Msg::OnchainFunds`
 * is emitted.
 */
class ChannelCreationDecider {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	ChannelCreationDecider() =delete;
	ChannelCreationDecider(ChannelCreationDecider const&) =delete;

	explicit
	ChannelCreationDecider(S::Bus& bus);
	ChannelCreationDecider(ChannelCreationDecider&&);
	~ChannelCreationDecider();
};

}}

#endif /* !defined(BOSS_MOD_CHANNELCREATIONDECIDER_HPP) */
