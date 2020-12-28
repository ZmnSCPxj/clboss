#include"Net/IPAddr.hpp"
#include"Net/IPAddrOrOnion.hpp"
#include<assert.h>
#include<cstdint>
#include<cstring>

namespace {

auto const onion_len = std::strlen(".onion");

bool is_dot_onion(std::string const& s) {
	if (s.size() < onion_len)
		return false;
	return std::strncmp(&*(s.end() - onion_len), ".onion", onion_len) == 0;
}

}

namespace Net {

class IPAddrOrOnion::Impl {
private:
	bool is_onion_flag;
	union {
		std::uint8_t onion_store[sizeof(std::string)];
		std::uint8_t ip_store[sizeof(Net::IPAddr)];
	} storage;

	std::string& as_onion() {
		assert(is_onion());
		return *reinterpret_cast<std::string*>(storage.onion_store);
	}
	std::string const& as_conion() const {
		assert(is_onion());
		return *reinterpret_cast<std::string const*>(storage.onion_store);
	}
	Net::IPAddr& as_ip() {
		assert(!is_onion());
		return *reinterpret_cast<Net::IPAddr*>(storage.ip_store);
	}
	Net::IPAddr const& as_cip() const {
		assert(!is_onion());
		return *reinterpret_cast<Net::IPAddr const*>(storage.ip_store);
	}

public:
	~Impl() {
		if (is_onion()) {
			/* GCC wants std::string::~string() but clang wants
			 * ~string.
			 * The using below works on both.
			 */
			using std::string;
			as_onion().~string();
		} else
			as_ip().~IPAddr();
	}
	Impl() {
		is_onion_flag = false;
		new(storage.ip_store) Net::IPAddr();
	}
	Impl(Impl const&) =delete;
	Impl(Impl&&) =delete;

	explicit
	Impl(std::string onion) {
		is_onion_flag = true;
		new(storage.onion_store) std::string(std::move(onion));
	}
	explicit
	Impl(Net::IPAddr ip) {
		is_onion_flag = false;
		new(storage.ip_store) Net::IPAddr(std::move(ip));
	}

	bool is_onion() const {
		return is_onion_flag;
	}
	void to_onion(std::string& onion) const {
		assert(is_onion());
		onion = as_conion();
	}
	void to_ip(Net::IPAddr& ip) const {
		assert(!is_onion());
		ip = as_cip();
	}
};

IPAddrOrOnion::IPAddrOrOnion(std::shared_ptr<Impl> pimpl_) : pimpl(std::move(pimpl_)) { }
IPAddrOrOnion::IPAddrOrOnion()
	: pimpl(std::make_shared<Impl>()) { }

IPAddrOrOnion::IPAddrOrOnion(std::string const& s) {
	if (is_dot_onion(s)) {
		pimpl = std::make_shared<Impl>(s);
		return;
	}
	try {
		auto ip = Net::IPAddr::v4(s);
		pimpl = std::make_shared<Impl>(std::move(ip));
	} catch (Net::IPAddrInvalid const& _) {
		auto ip = Net::IPAddr::v6(s);
		pimpl = std::make_shared<Impl>(std::move(ip));
	}
}

IPAddrOrOnion IPAddrOrOnion::from_ip_addr(IPAddr ip) {
	return IPAddrOrOnion(std::make_shared<Impl>(std::move(ip)));
}
IPAddrOrOnion IPAddrOrOnion::from_ip_addr_v4(std::string const& s) {
	return IPAddrOrOnion(std::make_shared<Impl>(IPAddr::v4(s)));
}
IPAddrOrOnion IPAddrOrOnion::from_ip_addr_v6(std::string const& s) {
	return IPAddrOrOnion(std::make_shared<Impl>(IPAddr::v6(s)));
}
IPAddrOrOnion IPAddrOrOnion::from_onion(std::string const& s) {
	if (!is_dot_onion(s))
		throw IPAddrInvalid(s);
	return IPAddrOrOnion(std::make_shared<Impl>(s));
}

bool IPAddrOrOnion::is_onion() const {
	return pimpl->is_onion();
}
void IPAddrOrOnion::to_ip_addr(Net::IPAddr& ip) const {
	pimpl->to_ip(ip);
}
void IPAddrOrOnion::to_onion(std::string& onion) const {
	pimpl->to_onion(onion);
}

IPAddrOrOnion::operator std::string() const {
	if (is_ip_addr()) {
		auto v = IPAddr();
		to_ip_addr(v);
		return std::string(v);
	} else {
		auto v = std::string();
		to_onion(v);
		return v;
	}
}

bool IPAddrOrOnion::operator==(Net::IPAddrOrOnion const& o) const {
	if (is_onion()) {
		if (!o.is_onion())
			return false;
		auto my_s = std::string();
		auto o_s = std::string();
		to_onion(my_s);
		o.to_onion(o_s);
		return my_s == o_s;
	} else {
		if (o.is_onion())
			return false;
		auto my_ip = Net::IPAddr();;
		auto o_ip = Net::IPAddr();;
		to_ip_addr(my_ip);
		o.to_ip_addr(o_ip);
		return my_ip == o_ip;
	}
}

}

std::ostream& operator<<(std::ostream& os, Net::IPAddrOrOnion const& n) {
	return os << std::string(n);
}
