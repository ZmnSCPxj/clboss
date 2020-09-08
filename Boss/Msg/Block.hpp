#ifndef BOSS_MSG_BLOCK_HPP
#define BOSS_MSG_BLOCK_HPP

#include<cstdint>

namespace Boss { namespace Msg {

/** struct Boss::Msg::Block
 *
 * @brief a new block has been processed by lightningd.
 *
 * @desc emitted at each new block.
 * While one block is being emitted, other Block messages
 * will not occur, and your module should see Blocks as
 * monotonically increasing.
 * However, subscribers which block will also delay other
 * subscribers (as is usual for S::Bus::raise).
 * Modules subscribed to this message should also note
 * that some heights can get skipped if blocks come in
 * faster than all CLBOSS modules can handle them.
 */
struct Block {
	std::uint32_t height;
};

}}

#endif /* !defined(BOSS_MSG_BLOCK_HPP) */
