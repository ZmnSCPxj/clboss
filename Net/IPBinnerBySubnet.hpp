#ifndef NET_IPBINNERBYSUBNET_HPP
#define NET_IPBINNERBYSUBNET_HPP

#include"Net/IPBinnerIF.hpp"

namespace Net {

/** class Net::IPBinnerBySubnet
 *
 * @brief an IP binner that bins by the first few
 * bytes of an IP address.
 */
class IPBinnerBySubnet : public IPBinnerIF {
public:
	IPBinnerBySubnet() =default;
	~IPBinnerBySubnet() =default;

	IPBinnerBySubnet(IPBinnerBySubnet const&) =default;
	IPBinnerBySubnet(IPBinnerBySubnet&&) =default;

	IPBin get_bin_of(Net::IPAddr const&) const override;
};

}

#endif /* !defined(NET_IPBINNERBYSUBNET_HPP) */
