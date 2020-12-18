#ifndef NET_IPBINNERIF_HPP
#define NET_IPBINNERIF_HPP

#include<cstdint>

namespace Net { class IPAddr; }

namespace Net {

typedef std::uint64_t IPBin;

/** class Net::IPBinnerIF
 *
 * @brief Abstract interface to an object that classifies IP addresses
 * to bins.
 */
class IPBinnerIF {
public:
	virtual ~IPBinnerIF() { }

	/* Get bin of the given IP address.  */
	virtual
	IPBin get_bin_of(Net::IPAddr const&) const =0;
};

}

#endif /* !defined(NET_IPBINNERIF_HPP) */
