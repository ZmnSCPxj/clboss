#include<netdb.h>
#include"Net/Detail/AddrInfoReleaser.hpp"

namespace Net { namespace Detail {

AddrInfoReleaser::~AddrInfoReleaser() {
	if (addrs)
		freeaddrinfo(addrs);
}

}}
