#ifndef BOSS_MSG_SOLICITCHANNELFEEMODIFIER_HPP
#define BOSS_MSG_SOLICITCHANNELFEEMODIFIER_HPP

namespace Boss { namespace Msg {

/** struct Boss::Msg::SolicitChannelFeeModifier
 *
 * @brief sent when the `Boss::Mod::ChannelFeeSetter`
 * wants to know how the other modules would like
 * fees to be modified.
 *
 * @desc Modules that want to modify the channel fees
 * should emit `Boss::Msg::ProvideChannelFeeModifier`
 * messages synchronously in response to this message.
 */
struct SolicitChannelFeeModifier {};

}}

#endif /* !defined(BOSS_MSG_SOLICITCHANNELFEEMODIFIER_HPP) */
