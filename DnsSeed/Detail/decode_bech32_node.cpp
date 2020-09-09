#include"DnsSeed/Detail/decode_bech32_node.hpp"
#include<algorithm>
#include<ctype.h>
#include<stdexcept>
#include<vector>

namespace {

auto const bech32_chars = std::string("qpzry9x8gf2tvdw0s3jn54khce6mua7l");

int decode_char(char c) {
	auto it = std::find( bech32_chars.begin(), bech32_chars.end()
			   , tolower(c)
			   );
	if (it == bech32_chars.end()) {
		char s[2];
		s[0] = c;
		s[1] = c;
		throw std::runtime_error(
			std::string("Unrecognized char: ") + s
		);
	}
	return it - bech32_chars.begin();
}

auto const binary = std::vector<std::vector<bool>>
{ {0, 0, 0, 0, 0}
, {0, 0, 0, 0, 1}
, {0, 0, 0, 1, 0}
, {0, 0, 0, 1, 1}
, {0, 0, 1, 0, 0}
, {0, 0, 1, 0, 1}
, {0, 0, 1, 1, 0}
, {0, 0, 1, 1, 1}
, {0, 1, 0, 0, 0}
, {0, 1, 0, 0, 1}
, {0, 1, 0, 1, 0}
, {0, 1, 0, 1, 1}
, {0, 1, 1, 0, 0}
, {0, 1, 1, 0, 1}
, {0, 1, 1, 1, 0}
, {0, 1, 1, 1, 1}
, {1, 0, 0, 0, 0}
, {1, 0, 0, 0, 1}
, {1, 0, 0, 1, 0}
, {1, 0, 0, 1, 1}
, {1, 0, 1, 0, 0}
, {1, 0, 1, 0, 1}
, {1, 0, 1, 1, 0}
, {1, 0, 1, 1, 1}
, {1, 1, 0, 0, 0}
, {1, 1, 0, 0, 1}
, {1, 1, 0, 1, 0}
, {1, 1, 0, 1, 1}
, {1, 1, 1, 0, 0}
, {1, 1, 1, 0, 1}
, {1, 1, 1, 1, 0}
, {1, 1, 1, 1, 1}
};

}

namespace DnsSeed { namespace Detail {

std::vector<std::uint8_t> decode_bech32_node(std::string const& s) {
	/* Get the separator.  */
	auto rit = std::find(s.rbegin(), s.rend(), '1');
	auto it = rit.base();

	if (it >= s.end() - 6)
		throw std::runtime_error(
			std::string("Data part too short: ") + s
		);

	auto bitstream = std::vector<bool>();
	std::for_each( it, s.end() - 6
		     , [&](char c) {
		auto val = decode_char(c);
		auto const& bin = binary[val];
		std::copy( bin.begin(), bin.end()
			 , std::back_inserter(bitstream)
			 );
	});
	/* 264 bits needed, but bech32 encodes in batches of 5 bits so
	 * we round up to 265.  */
	if (bitstream.size() != 33 * 8 + 1)
		throw std::runtime_error(
			std::string("Data part wrong size: ") + s
		);
	/* The last bit should be 0.  */
	if (bitstream[264] == 1)
		throw std::runtime_error(
			std::string("Odd node id: ") + s
		);
	/* And should be removed.  */
	bitstream.erase(bitstream.end() - 1);

	auto rv = std::vector<std::uint8_t>(33);
	for (auto i = size_t(0); i < 33; ++i) {
		auto byte = std::uint8_t(0);
		std::for_each( bitstream.begin() + i * 8
			     , bitstream.begin() + (i + 1) * 8
			     , [&](bool b) {
			byte = byte << 1;
			byte |= b ? 1 : 0;
		});
		rv[i] = byte;
	}

	return rv;
}

}}
