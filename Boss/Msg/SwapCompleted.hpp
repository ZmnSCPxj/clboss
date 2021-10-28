#ifndef BOSS_MSG_SWAPCOMPLETED_HPP
#define BOSS_MSG_SWAPCOMPLETED_HPP

#include"Ln/Amount.hpp"
#include<memory>

namespace Sqlite3 { class Tx; }

namespace Boss { namespace Msg {

/** struct Boss::Msg::SwapCompleted
 *
 * @brief broadcasted whenever the `SwapManager` has
 * finished a swap.
 */
struct SwapCompleted {
	/* Database transaction initiated by the `SwapManager`.
	 * It will be committed by the `SwapManager` after the
	 * broadcast.
	 */
	std::shared_ptr<Sqlite3::Tx> dbtx;

	/* The amount that was swapped out.  */
	Ln::Amount amount_sent;
	/* The amount we got.  */
	Ln::Amount amount_received;

	/* The swap provider name.  */
	std::string provider_name;
};

}}

#endif /* !defined(BOSS_MSG_SWAPCOMPLETED_HPP) */
