#ifndef BOSS_MSG_REQUESTCHANNELCREATION_HPP
#define BOSS_MSG_REQUESTCHANNELCREATION_HPP

#include"Ln/Amount.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::RequestChannelCreation
 *
 * @brief emitted when we think it is a good
 * idea to create channels now.
 * It contains the total amount of bitcoins
 * to target to put into channels.
 */
struct RequestChannelCreation {
	Ln::Amount amount;
};

}}

#endif /* BOSS_MSG_REQUESTCHANNELCREATION_HPP */
