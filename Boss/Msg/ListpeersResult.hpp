#ifndef BOSS_MSG_LISTPEERSRESULT_HPP
#define BOSS_MSG_LISTPEERSRESULT_HPP

#include"Jsmn/Object.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ListpeersResult
 *
 * @brief announced during `init` to inform all
 * modules about `listpeers` command result.
 * Also announced every 10 minutes.
 */
struct ListpeersResult {
	/* Known to be an array.  */
	Jsmn::Object peers;
	/* Whether this listpeers was performed during `init`
	 * or on the 10-minute timer.
	 */
	bool initial;
};

}}

#endif /* !defined(BOSS_MSG_LISTPEERSRESULT_HPP) */
