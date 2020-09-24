#include"Bitcoin/addr_to_scriptPubKey.hpp"
#include"Util/Bech32.hpp"
#include<iterator>

namespace Bitcoin {

std::vector<std::uint8_t>
addr_to_scriptPubKey(std::string const& addr) {
	auto hrp = std::string();
	auto data = std::vector<bool>();
	if (Util::Bech32::decode(hrp, data, addr)) {
		/* TODO: Check the network matches.  */
		if ( hrp != "bc"
		  && hrp != "tb"
		  && hrp != "bcrt"
		   )
			throw UnknownAddrType();

		if (data.size() < 5)
			throw UnknownAddrType();

		/* Decode the segwit version.  */
		auto segwit_version = std::uint8_t();
		Util::Bech32::bitstream_to_bytes( data.begin()
						, data.begin() + 5
						, &segwit_version
						);
		/* Remove the 3 bits padding.  */
		segwit_version = segwit_version >> 3;

		/* Check on segwit version.  */
		auto ok = ( segwit_version == 0
		         && ( data.size() == 5 + 160
			   || data.size() == 5 + 260
			    )
			  )
		       || ( segwit_version == 1
			 && data.size() == 5 + 260
			  )
		       || ( 2 <= segwit_version && segwit_version <= 16);
		if (!ok)
			throw UnknownAddrType();

		/* Get the witness program size in bytes.  */
		auto witness_program_size = (data.size() - 5) / 8;
		/* From 1 to 75 bytes.  */
		if (!(1 <= witness_program_size && witness_program_size <= 75))
			throw UnknownAddrType();

		/* Encode.  */
		auto rv = std::vector<std::uint8_t>();
		rv.push_back(segwit_version == 0 ? 0x00
						 : (0x50 | segwit_version)
						 );
		rv.push_back(std::uint8_t(witness_program_size));
		Util::Bech32::bitstream_to_bytes( data.begin() + 5
						, data.begin() + 5
						  + (8 * witness_program_size)
						, std::back_inserter(rv)
						);
		return rv;
	} else {
		/* TODO: Base58 support.  */
		throw UnknownAddrType();
	}
}

}
