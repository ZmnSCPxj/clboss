#ifndef BOSS_MSG_CHANNELFUNDS_HPP
#define BOSS_MSG_CHANNELFUNDS_HPP

#include"Ln/Amount.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ChannelFunds
 *
 * @brief periodically emitted to inform all modules
 * of channel funds.
 */
struct ChannelFunds {
	/* Total on all channels.  */
	Ln::Amount total;
	/* Total on connected channels.  */
	Ln::Amount connected;
};

}}

#endif /* !defined(BOSS_MSG_CHANNELFUNDS_HPP) */
