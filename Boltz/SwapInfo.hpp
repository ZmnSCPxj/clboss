#ifndef BOLTZ_SWAPINFO_HPP
#define BOLTZ_SWAPINFO_HPP

#include"Sha256/Hash.hpp"
#include<cstdint>
#include<string>

namespace Boltz {

/** struct Boltz::SwapInfo
 *
 * @brief Contains information about the swap.
 */
struct SwapInfo {
	/* Pay this in order to complete the swap.  */
	std::string invoice;
	/* The hash of the above invoice.  */
	Sha256::Hash hash;
	/* Blockheight.  */
	std::uint32_t timeout;
};

}

#endif /* !defined(BOLTZ_SWAPINFO_HPP) */
