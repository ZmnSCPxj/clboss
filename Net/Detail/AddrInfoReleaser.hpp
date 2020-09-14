#ifndef NET_DETAIL_ADDRINFORELEASER_HPP
#define NET_DETAIL_ADDRINFORELEASER_HPP

extern "C" { struct addrinfo; }

namespace Net { namespace Detail {

/* RAII class to release addrinfo structures.  */
class AddrInfoReleaser {
private:
	addrinfo* addrs;

public:
	AddrInfoReleaser() : addrs(nullptr) { }
	AddrInfoReleaser(AddrInfoReleaser const&) =delete;
	AddrInfoReleaser(AddrInfoReleaser&& o) {
		addrs = o.addrs;
		o.addrs = nullptr;
	}
	~AddrInfoReleaser();
	addrinfo*& get() { return addrs; }
};

}}

#endif /* NET_DETAIL_ADDRINFORELEASER_HPP */
