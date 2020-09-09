#ifndef DNSSEED_DETAIL_DECODE_BECH32_NODE_HPP
#define DNSSEED_DETAIL_DECODE_BECH32_NODE_HPP

#include<cstdint>
#include<string>
#include<vector>

namespace DnsSeed { namespace Detail {

/** DnsSeed::Detail::decode_bech32_node
 *
 * @brief decode a bech32-encoded node ID.
 *
 * @desc Decode a bech32-encoded node ID, returning
 * the raw Lightning node ID.
 *
 * WARNING this does not actually check the checksum.
 * Our assumption is that these pieces of text come from
 * DNS, not from a human that might make mistakes, and
 * that the DNS server is trusted to give the correct
 * public key anyway.
 *
 * Throws an std::runtime_error in case of a parse
 * error.
 */
std::vector<std::uint8_t> decode_bech32_node(std::string const&);

}}

#endif /* !defined(DNSSEED_DETAIL_DECODE_BECH32_NODE_HPP) */
