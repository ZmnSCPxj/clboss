#ifndef UTIL_STR_HPP
#define UTIL_STR_HPP

/*
 * Minor string utilities.
 */

#include<cstdint>
#include<stdexcept>
#include<string>
#include<vector>

namespace Util {
namespace Str {

/* Outputs a two-digit hex string of the given byte.  */
std::string hexbyte(std::uint8_t);
/* Outputs a string of the given data.  */
std::string hexdump(void const* p, std::size_t s);

/* Creates a buffer from the given hex string.  */
/* FIXME: use a backtrace-extracting exception. */
struct HexParseFailure : public std::runtime_error {
	HexParseFailure(std::string msg)
		: std::runtime_error("hexread: " + msg) { }
};
std::vector<std::uint8_t> hexread(std::string const&);

/* Checks that the given string is a hex string with an
 * even number of digits.
 */
bool ishex(std::string const&);

std::string trim(std::string const& s);

}}

#endif /* !defined(UTIL_STR_HPP) */
