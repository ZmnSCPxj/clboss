#include<errno.h>
#include<iomanip>
#include<netdb.h>
#include<sstream>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include"Net/Detail/AddrInfoReleaser.hpp"
#include"Net/DirectConnector.hpp"
#include"Net/Fd.hpp"
#include"Net/SocketFd.hpp"
#include"Util/make_unique.hpp"

namespace {

std::string stringify_int(int port) {
	auto os = std::ostringstream();
	os << std::dec << port;
	return os.str();
}

}

namespace Net {

Net::SocketFd
DirectConnector::connect(std::string const& host, int port) {
	auto portstring = stringify_int(port);

	auto hint = addrinfo();
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_UNSPEC;       /* IPv4 or IPv6, either is fine.  */
	hint.ai_socktype = SOCK_STREAM;   /* TCP interface.  */
	hint.ai_protocol = 0;             /* Any protocol.  */
	hint.ai_flags = AI_ADDRCONFIG     /* Use IPv6 only if we have IPv6 ourselves. */
		      ;

	auto addrs = Detail::AddrInfoReleaser();
	auto res = getaddrinfo( host.c_str(), portstring.c_str()
			      , &hint
			      , &addrs.get()
			      );
	/* FIXME: Return or log the exact failure?  */
	if (res < 0)
		return nullptr;

	for (auto p = addrs.get(); p; p = p->ai_next) {
		auto sfd = Fd(socket( p->ai_family
				    , p->ai_socktype
				    , p->ai_protocol
				    ));
		/* If bad, try next.  */
		if (!sfd)
			continue;

		auto cres = int();
		do {
			cres = ::connect( sfd.get()
					, p->ai_addr
					, p->ai_addrlen
					);
		} while (cres < 0 && errno == EINTR);
		if (cres == 0)
			return SocketFd(std::move(sfd));

		/* If bad, just continue.  */
	}

	/* Everything failed.  */
	return nullptr;
}

}
