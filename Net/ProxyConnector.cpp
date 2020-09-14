#include<errno.h>
#include<netdb.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include"Net/Detail/AddrInfoReleaser.hpp"
#include"Net/Fd.hpp"
#include"Net/ProxyConnector.hpp"
#include"Net/SocketFd.hpp"
#include"Util/Rw.hpp"

namespace {

bool writebuff( Net::SocketFd const& fd
	      , std::vector<std::uint8_t> const& data
	      ) {
	return Util::Rw::write_all( fd.get()
				  , &data[0]
				  , data.size()
				  );
}
bool readbuff( Net::SocketFd const& fd
	     , std::vector<std::uint8_t>& data
	     ) {
	auto size = data.size();
	return Util::Rw::read_all( fd.get()
				 , &data[0]
				 , size
				 );
}

enum HostType
{ IPv4
, IPv6
, Name
};

void analyze_host( std::string const& host
		 , HostType& addr_type
		 , std::vector<std::uint8_t>& addr_data
		 ) {
	auto hint = addrinfo();
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv5.  */
	hint.ai_socktype = SOCK_STREAM; /* TCP.  */
	hint.ai_protocol = 0; /* Any protocol.  */
	hint.ai_flags = AI_NUMERICHOST;

	auto addrs = Net::Detail::AddrInfoReleaser();
	auto res = getaddrinfo( host.c_str(), nullptr
			      , &hint
			      , &addrs.get()
			      );
	auto& paddr = addrs.get();

	if (res == 0 && paddr && paddr->ai_family == AF_INET) {
		addr_type = IPv4;
		auto sa = reinterpret_cast<sockaddr_in*>(paddr->ai_addr);
		addr_data.resize(4);
		memcpy(&addr_data[0], &sa->sin_addr, addr_data.size());
		return;
	}
	if (res == 0 && paddr && paddr->ai_family == AF_INET6) {
		addr_type = IPv6;
		auto sa = reinterpret_cast<sockaddr_in6*>(paddr->ai_addr);
		addr_data.resize(16);
		memcpy(&addr_data[0], &sa->sin6_addr, addr_data.size());
		return;
	}

	/* If reached here, assume failed to lookup, or some newfangled
	 * IP type.
	 */
	addr_type = Name;
	addr_data.resize(host.length() + 1);
	if (addr_data.size() > 256)
		addr_data.resize(256);
	addr_data[0] = std::uint8_t(addr_data.size() - 1);
	memcpy(&addr_data[1], &host[0], addr_data.size() - 1);
	return;

}

int to_errno(std::uint8_t socks5_err) {
	switch (socks5_err) {
	case 0x00: return 0;
	case 0x01: return ENETDOWN;
	case 0x02: return EACCES;
	case 0x03: return ENETUNREACH;
	case 0x04: return EHOSTUNREACH;
	case 0x05: return ECONNREFUSED;
	case 0x06: return ETIMEDOUT;
	case 0x07: return EINVAL;
	case 0x08: return EAFNOSUPPORT;
	default: return EPROTO;
	}
}

}

namespace Net {

Net::SocketFd
ProxyConnector::connect(std::string const& host, int port) {
	/* Determine if host is a IPv4 or IPv6 numeric address.  */
	auto hosttype = HostType();
	auto hostaddr = std::vector<std::uint8_t>();
	analyze_host(host, hosttype, hostaddr);

	auto ret = base->connect(proxy_host, proxy_port);
	if (!ret)
		return ret;

	auto buffer = std::vector<std::uint8_t>();

	/* RFC1928.  */

	/* Greeting.  */
	buffer.resize(3);
	buffer[0] = 0x05; /* SOCKS5.  */
	buffer[1] = 0x01; /* One authentication method.  */
	buffer[2] = 0x00; /* Unauthenticated method.  */
	if (!writebuff(ret, buffer)) {
		ret.reset();
		return ret;
	}
	/* Server-selected authentication.  */
	buffer.resize(2);
	if (!readbuff(ret, buffer)) {
		ret.reset();
		return ret;
	}
	if (buffer[0] != 0x05) {
		/* Ummmmm not SOCKS5?  */
		ret.reset();
		errno = EPROTO;
		return ret;
	}
	if (buffer[1] != 0x00) {
		/* Authentication denied!  */
		ret.reset();
		errno = EACCES;
		return ret;
	}

	/* Request.  */
	buffer.resize(4);
	buffer[0] = 0x05; /* SOCKS5.  */
	buffer[1] = 0x01; /* CONNECT.  */
	buffer[2] = 0x00; /* RSV.  */
	switch (hosttype) {
	/* ATYP.  */
	case IPv4: buffer[3] = 0x01; break;
	case Name: buffer[3] = 0x03; break;
	case IPv6: buffer[3] = 0x04; break;
	}
	if (!writebuff(ret, buffer)) {
		ret.reset();
		return ret;
	}
	if (!writebuff(ret, hostaddr)) {
		ret.reset();
		return ret;
	}
	/* Port.  */
	buffer.resize(2);
	buffer[0] = std::uint8_t((port >> 8) & 0xFF);
	buffer[1] = std::uint8_t((port >> 0) & 0xFF);
	if (!writebuff(ret, buffer)) {
		ret.reset();
		return ret;
	}

	/* Response.  */
	buffer.resize(4);
	if (!readbuff(ret, buffer)) {
		ret.reset();
		return ret;
	}
	/* Check VER and RSV.  */
	if (buffer[0] != 0x05 || buffer[2] != 0x0) {
		ret.reset();
		errno = EPROTO;
		return ret;
	}
	/* REP.  */
	if (buffer[1] != 0x0) {
		ret.reset();
		errno = to_errno(buffer[1]);
		return ret;
	}
	/* Parse ATYP.  */
	switch (buffer[3]) {
	case 0x01:
		buffer.resize(4);
		break;
	case 0x03:
		buffer.resize(1);
		if (!readbuff(ret, buffer)) {
			ret.reset();
			return ret;
		}
		buffer.resize(std::size_t(buffer[0]));
		break;
	case 0x04:
		buffer.resize(16);
		break;
	default:
		ret.reset();
		errno = EPROTO;
		return ret;
	}
	/* BND.ADDR.  */
	if (!readbuff(ret, buffer)) {
		ret.reset();
		return ret;
	}
	/* BND.PORT.  */
	buffer.resize(2);
	if (!readbuff(ret, buffer)) {
		ret.reset();
		return ret;
	}

	/* Completed!  */

	return ret;
}

}
