#ifndef BOSS_MSG_PROVIDECHANNELFEEMODIFIERDETAILED_HPP
#define BOSS_MSG_PROVIDECHANNELFEEMODIFIERDETAILED_HPP

#include<functional>
#include<utility>

namespace Ev { template<typename a> class Io; }
namespace Ln { class NodeId; }

namespace Boss { namespace Msg {

/** struct Boss::Msg::ProvideChannelFeeModifierDetailed
 *
 * @brief emitted in response to the `Boss::Msg::SolicitChannelFeeModifier`
 * message.
 *
 * @desc the function you put into this message is given the node ID,
 * plus the raw base median feerate and the raw proportional median
 * feerate.
 *
 * The given function must then return a pair of `double`s, the `.first`
 * of which is multiplied to the `base` median fee, the `.second` of
 * which is multiplied to the `proportional` median fee.
 */
struct ProvideChannelFeeModifierDetailed {
	std::function< Ev::Io<std::pair< double /* base_multiplier */
				       , double /* proportional_multiplier */
				       >>( Ln::NodeId
					 , std::uint32_t /* base */
					 , std::uint32_t /* proportional */
					 )
		     > modifier;
};

}}

#endif /* !defined(BOSS_MSG_PROVIDECHANNELFEEMODIFIERDETAILED_HPP) */
