#include"Net/IPAddr.hpp"
#include"Net/IPBinnerBySubnet.hpp"

namespace {

/* How many bytes to get for IPv4 and IPv6.  */
auto prefix_length_4 = std::size_t(2);
auto prefix_length_6 = std::size_t(3);

}

namespace Net {

IPBin IPBinnerBySubnet::get_bin_of(Net::IPAddr const& ip) const {
	auto& data = ip.raw_data();
	auto prefix_length = ip.is_ipv4() ? prefix_length_4 : prefix_length_6;

	auto rv = std::uint64_t(0);
	for (auto i = std::size_t(0); i < prefix_length; ++i) {
		rv = rv << 8;
		if (i < data.size())
			rv += std::uint64_t(data[i]);
	}

	return rv;
}

}
