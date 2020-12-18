#include"Net/IPAddr.hpp"
#include<arpa/inet.h>
#include<netdb.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>

namespace Net {

class IPAddr::Impl {
private:
	std::vector<std::uint8_t> data;
	bool ipv4;

public:
	void construct_by_v4(std::string const& s) {
		data.resize(sizeof(struct in_addr));
		auto res = inet_pton(AF_INET, s.c_str(), (void*) &data[0]);
		if (!res)
			throw IPAddrInvalid(s);
		ipv4 = true;
	}
	void construct_by_v6(std::string const& s) {
		data.resize(sizeof(struct in6_addr));
		auto res = inet_pton(AF_INET6, s.c_str(), (void*) &data[0]);
		if (!res)
			throw IPAddrInvalid(s);
		ipv4 = false;
	}

	bool is_ipv4() const {
		return ipv4;
	}
	std::vector<std::uint8_t> const& raw_data() const {
		return data;
	}
	bool operator==(Impl const& o) const {
		if (this == &o)
			return true;
		return data == o.data;
	}

	operator std::string() const {
		if (ipv4) {
			char buff[INET_ADDRSTRLEN + 1];
			inet_ntop(AF_INET, (void*) &data[0], buff, sizeof(buff));
			return std::string(buff);
		} else {
			char buff[INET6_ADDRSTRLEN + 1];
			inet_ntop(AF_INET6, (void*) &data[0], buff, sizeof(buff));
			return std::string(buff);
		}
	}
};

IPAddr::IPAddr() : pimpl(std::make_shared<Impl>()) {
	pimpl->construct_by_v4("127.0.0.1");
}
IPAddr::IPAddr(std::shared_ptr<Impl> pimpl_) : pimpl(std::move(pimpl_)) { }

IPAddr IPAddr::v4(std::string const& ip) {
	auto pimpl = std::make_shared<Impl>();
	pimpl->construct_by_v4(ip);
	return IPAddr(std::move(pimpl));
}
IPAddr IPAddr::v6(std::string const& ip) {
	auto pimpl = std::make_shared<Impl>();
	pimpl->construct_by_v6(ip);
	return IPAddr(std::move(pimpl));
}
bool IPAddr::is_ipv4() const {
	return pimpl->is_ipv4();
}
std::vector<std::uint8_t> const& IPAddr::raw_data() const {
	return pimpl->raw_data();
}
bool IPAddr::operator==(IPAddr const& o) const {
	return *pimpl == *o.pimpl;
}
IPAddr::operator std::string() const {
	return std::string(*pimpl);
}

}

std::ostream& operator<<(std::ostream& os, Net::IPAddr const& ip) {
	return os << std::string(ip);
}
