#ifndef BOSS_MSG_LISTFUNDSANALYZEDRESULT_HPP
#define BOSS_MSG_LISTFUNDSANALYZEDRESULT_HPP

#include"Ln/Amount.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::ListfundsAnalyzedResult
 *
 * @brief The result from `listfunds`, analyzed.
 */
struct ListfundsAnalyzedResult {
	/* Total funds owned by us, both onchain and offchain.  */
	Ln::Amount total_owned;
};

}}

#endif /* !defined(BOSS_MSG_LISTFUNDSANALYZEDRESULT_HPP) */
