#ifndef BOLTZ_DETAIL_FIND_LOCKUP_OUTNUM_HPP
#define BOLTZ_DETAIL_FIND_LOCKUP_OUTNUM_HPP

#include<cstdint>
#include<vector>

namespace Bitcoin { class Tx; }

namespace Boltz { namespace Detail {

/** Boltz::Detail::find_lockup_outnum
 *
 * @brief finds the output of the given
 * transaction which pays out to a P2WSH
 * that matches the given `redeemScript`.
 *
 * @return -1 if not found, or a zero or
 * positive output index if found.
 */
int find_lockup_outnum( Bitcoin::Tx const& tx
		      , std::vector<std::uint8_t> const& redeemScript
		      );

}}

#endif /* !defined(BOLTZ_DETAIL_FIND_LOCKUP_OUTNUM_HPP) */
