#ifndef BITCOIN_ADDR_TO_SCRIPTPUBKEY_HPP
#define BITCOIN_ADDR_TO_SCRIPTPUBKEY_HPP

#include"Util/BacktraceException.hpp"
#include<cstdint>
#include<string>
#include<stdexcept>
#include<vector>

namespace Bitcoin {

struct UnknownAddrType : public Util::BacktraceException<std::invalid_argument> {
public:
	UnknownAddrType()
		: Util::BacktraceException<std::invalid_argument>("Bitcoin::UnknownAddrType") { }
};

/** Bitcoin::addr_to_scriptPubKey
 *
 * @brief given an address, returns the
 * `scriptPubKey` it should have, or
 * throws `Bitcoin::UnknownAddrType`
 * if address format fails.
 *
 * @desc Our assumption is that this
 * software only talks to other software,
 * and not to mistake-prone silly humans,
 * so this function does ***not*** do any
 * kind of checksum validation.
 */
std::vector<std::uint8_t>
addr_to_scriptPubKey(std::string const&);

}

#endif /* !defined(BITCOIN_ADDR_TO_SCRIPTPUBKEY_HPP) */
