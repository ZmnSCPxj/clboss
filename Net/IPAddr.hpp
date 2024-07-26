#ifndef NET_IPADDR_HPP
#define NET_IPADDR_HPP

#include"Util/BacktraceException.hpp"
#include<cstdint>
#include<memory>
#include<ostream>
#include<stdexcept>
#include<string>
#include<vector>

namespace Net { class IPAddr; }
std::ostream& operator<<(std::ostream&, Net::IPAddr const&);

namespace Net {

/** class Net::IPAddrInvalid.
 *
 * @brief thrown when an invalid IP address is given.
 */
class IPAddrInvalid : public Util::BacktraceException<std::invalid_argument> {
public:
	std::string invalid_ip;
	explicit IPAddrInvalid( std::string const& invalid_ip_
		              ) : Util::BacktraceException<std::invalid_argument>( std::string("Invalid IP: ") 
						       + invalid_ip_
						       )
				, invalid_ip(invalid_ip_)
				{ }
};

/** class Net::IPAddr
 *
 * @brief representation for IP addresses,
 * either IPv4 or IPv6.
 */
class IPAddr {
private:
	class Impl;
	std::shared_ptr<Impl> pimpl;

	explicit
	IPAddr(std::shared_ptr<Impl> pimpl_);

public:
	/* Set to 127.0.0.1 */
	IPAddr();

	IPAddr(IPAddr const&) =default;
	IPAddr(IPAddr&&) =default;
	IPAddr& operator=(IPAddr const&) =default;
	IPAddr& operator=(IPAddr&&) =default;

	/* Specific factories.  */
	static
	IPAddr v4(std::string const&);
	static
	IPAddr v6(std::string const&);

	/* Queries.  */
	bool is_ipv4() const;
	bool is_ipv6() const { return !is_ipv4(); }
	std::vector<std::uint8_t> const& raw_data() const;
	/* Comparison.  */
	bool operator==(IPAddr const&) const;
	bool operator!=(IPAddr const& o) const {
		return !(*this == o);
	}

	/* Printing.  */
	explicit
	operator std::string() const;

	friend std::ostream& ::operator<<(std::ostream&, Net::IPAddr const&);
};

}

#endif /* !defined(NET_IPADDR_HPP) */
