#include"Util/Bech32.hpp"
#include<algorithm>
#include<assert.h>
#include<ctype.h>

namespace {

auto const bech32_chars = std::string("qpzry9x8gf2tvdw0s3jn54khce6mua7l");

int decode_char(char c) {
	auto it = std::find( bech32_chars.begin(), bech32_chars.end()
			   , tolower(c)
			   );
	if (it == bech32_chars.end()) {
		return -1;
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

namespace Util { namespace Bech32 {

bool decode( std::string& hrp
	   , std::vector<bool>& data
	   , std::string const& bech32
	   ) {
	/* Get the separator.
	 * The last `1` character is the separator, so
	 * do the scan in reverse order.
	 */
	auto rit = std::find(bech32.rbegin(), bech32.rend(), '1');
	/* No `1` separator.  */
	if (rit == bech32.rend())
		return false;
	/* Convert to forward iterator.  */
	auto it = rit.base();

	/* Data part too short.  */
	if (it >= bech32.end() - 6)
		return false;

	/* `it` is *after* the `1` separator.
	 * The HRP is up to but not including the `1` separator,
	 * so we use `it - 1` as the end to point to the `1` separator.
	 */
	assert(it != bech32.begin());
	hrp.resize(it - 1 - bech32.begin());
	std::transform( bech32.begin(), it - 1
		      , hrp.begin()
		      , [](char c) {
		return tolower(c);
	});

	/* Load the data.  */
	data = std::vector<bool>();
	for (auto p = it; p < bech32.end() - 6; ++p) {
		auto val = decode_char(*p);
		if (val < 0)
			/* Non-bech32 char.  */
			return false;
		auto const& bin = binary[val];
		std::copy( bin.begin(), bin.end()
			 , std::back_inserter(data)
			 );
	}

	return true;
}

}}
