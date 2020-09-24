#include"DnsSeed/Detail/decode_bech32_node.hpp"
#include"Util/Bech32.hpp"
#include<algorithm>
#include<ctype.h>
#include<stdexcept>
#include<vector>

namespace DnsSeed { namespace Detail {

std::vector<std::uint8_t> decode_bech32_node(std::string const& s) {
	auto hrp = std::string();
	auto bitstream = std::vector<bool>();

	if (!Util::Bech32::decode(hrp, bitstream, s))
		throw std::runtime_error(
			std::string("Not a bech32 string: ") + s
		);

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
	Util::Bech32::bitstream_to_bytes( bitstream.begin(), bitstream.end()
					, rv.begin()
					);

	return rv;
}

}}
