#ifndef NET_IPADDRORONION_HPP
#define NET_IPADDRORONION_HPP

#include<memory>
#include<string>

namespace Net { class IPAddr; }
namespace Net { class IPAddrOrOnion; }
std::ostream& operator<<(std::ostream&, Net::IPAddrOrOnion const&);

namespace Net {

/** class Net::IPAddrOrOnion
 *
 * @brief representation for an access point for a
 * node.
 */
class IPAddrOrOnion {
private:
	class Impl;
	std::shared_ptr<Impl> pimpl;

	explicit
	IPAddrOrOnion(std::shared_ptr<Impl> pimpl_);

public:
	/* Set to IP 127.0.0.1 */
	IPAddrOrOnion();

	/* Autodetect string.  */
	explicit
	IPAddrOrOnion(std::string const&);

	IPAddrOrOnion(IPAddrOrOnion const&) =default;
	IPAddrOrOnion(IPAddrOrOnion&&) =default;
	IPAddrOrOnion& operator=(IPAddrOrOnion const&) =default;
	IPAddrOrOnion& operator=(IPAddrOrOnion&&) =default;

	/* Explicit factories.  */
	static
	IPAddrOrOnion from_ip_addr(IPAddr);
	static
	IPAddrOrOnion from_ip_addr_v4(std::string const&);
	static
	IPAddrOrOnion from_ip_addr_v6(std::string const&);
	static
	IPAddrOrOnion from_onion(std::string const&);

	/* Printing.  */
	explicit
	operator std::string() const;

	/* Destructuring.  */
	bool is_ip_addr() const { return !is_onion(); }
	void to_ip_addr(IPAddr& ip) const;
	bool is_ip_addr(IPAddr& ip) const {
		auto rv = is_ip_addr();
		if (rv)
			to_ip_addr(ip);
		return rv;
	}
	bool is_onion() const;
	void to_onion(std::string& onion) const;
	bool is_onion(std::string& onion) const {
		auto rv = is_onion();
		if (rv)
			to_onion(onion);
		return rv;
	}

	/* Equality and inequality check.  */
	bool operator==(Net::IPAddrOrOnion const& o) const;
	bool operator!=(Net::IPAddrOrOnion const& o) const {
		return !(*this == o);
	}

	friend std::ostream& ::operator<<(std::ostream&, Net::IPAddrOrOnion const&);
};

}

#endif /* !defined(NET_IPADDRORONION_HPP) */
