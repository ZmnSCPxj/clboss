#ifndef BOSS_MOD_CHANNELFEEMANAGER_HPP
#define BOSS_MOD_CHANNELFEEMANAGER_HPP

#include"Ln/NodeId.hpp"
#include<cstdint>
#include<functional>
#include<map>
#include<vector>

namespace Ev { template<typename a> class Io; }
namespace S { class Bus; }

namespace Boss { namespace Mod {

/** class Boss::Mod::ChannelFeeManager
 *
 * @brief module that sets the channel fees, based
 * on the median fees of other peers of the node,
 * multiplied by modifiers solicited from other
 * modules.
 */
class ChannelFeeManager {
private:
	S::Bus& bus;

	bool initted;
	bool soliciting;
	std::vector<std::function< Ev::Io<double>( Ln::NodeId
						 , std::uint32_t // b
						 , std::uint32_t // p
						 )
				 >> modifiers;

	enum ZeroBaseFee {
		ZeroBaseFee_Require,
		ZeroBaseFee_Allow,
		ZeroBaseFee_Disallow
	};
	ZeroBaseFee zero_base_fee;

	void start();
	Ev::Io<void> perform( Ln::NodeId node
			    , std::uint32_t median_base
			    , std::uint32_t median_proportional
			    );
	Ev::Io<void> solicit();

	struct Info {
		std::uint32_t median_base;
		std::uint32_t median_proportional;
		double multiplier;
		std::uint32_t final_base;
		std::uint32_t final_proportional;
	};
	std::map<Ln::NodeId, Info> infos;

public:
	ChannelFeeManager() =delete;
	ChannelFeeManager(ChannelFeeManager&&) =delete;
	ChannelFeeManager(ChannelFeeManager const&) =delete;

	explicit
	ChannelFeeManager(S::Bus& bus_
			) : bus(bus_)
			  , initted(false)
			  , soliciting(false)
			  , zero_base_fee(ZeroBaseFee_Allow)
			  { start(); }
};

}}

#endif /* !defined(BOSS_MOD_CHANNELFEEMANAGER_HPP) */
