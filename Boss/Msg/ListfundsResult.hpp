#ifndef BOSS_MSG_LISTFUNDSRESULT_HPP
#define BOSS_MSG_LISTFUNDSRESULT_HPP

#include"Jsmn/Object.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ListfundsResult
 *
 * @brief periodically emitted with the result
 * of `listfunds`.
 */
struct ListfundsResult {
	/* Known to be an array.  */
	Jsmn::Object outputs;
	/* Known to be an array.  */
	Jsmn::Object channels;
};

}}

#endif /* !defined(BOSS_MSG_LISTFUNDSRESULT_HPP) */
