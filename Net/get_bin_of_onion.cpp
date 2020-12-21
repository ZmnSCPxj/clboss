#include"Net/get_bin_of_onion.hpp"
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"

namespace Net {

IPBin get_bin_of_onion( std::function<void(std::uint8_t hash[32])> tweak_hash
		      , std::string const& onion
		      ) {
	auto hash = Sha256::fun(&onion[0], onion.size());
	std::uint8_t buff[32];
	hash.to_buffer(buff);
	if (tweak_hash)
		tweak_hash(buff);
	return 0xFFFFFFFFFFFFFFFC + (buff[0] & 0x03);
}

}
