#ifndef DNSSEED_DETAIL_PARSE_DIG_SRV_HPP
#define DNSSEED_DETAIL_PARSE_DIG_SRV_HPP

#include<cstdint>
#include<string>
#include<vector>

namespace DnsSeed { namespace Detail {

struct Record {
	std::string nodeid;
	std::uint16_t port;
	std::string hostname;
};

/** DnsSeed::Detail::parse_dig_srv
 *
 * @brief parses the output of a `dig SRV` command, extracting
 * the node ID, port, and hostname of each entry returned.
 */
std::vector<Record> parse_dig_srv(std::string const&);

}}

#endif /* !defined(DNSSEED_DETAIL_PARSE_DIG_SRV_HPP) */
