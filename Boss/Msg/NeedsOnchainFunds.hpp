#ifndef BOSS_MSG_NEEDSONCHAINFUNDS_HPP
#define BOSS_MSG_NEEDSONCHAINFUNDS_HPP

#include"Ln/Amount.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::NeedsOnchainFunds
 *
 * @brief Broadcasted whenever we have onchain
 * funds, but those are too low to practically
 * put into reasonable-sized channels.
 * This message contains the missing amount.
 */
struct NeedsOnchainFunds {
	Ln::Amount needed;
};

}}

#endif /* !defined(BOSS_MSG_NEEDSONCHAINFUNDS_HPP) */
