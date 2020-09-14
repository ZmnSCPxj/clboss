#ifndef NET_CONNECTOR_HPP
#define NET_CONNECTOR_HPP

#include<string>

namespace Net { class SocketFd; }

namespace Net {

/* Interface to an entity that is capable of connecting
 * to a host:port.
 * This is an abstract interface since we might want to
 * use a proxy of some kind.
 */
class Connector {
public:
	virtual ~Connector() { }

	/* Returns null if failed to connect.  */
	virtual
	Net::SocketFd
	connect(std::string const& host, int port) =0;
};

}

#endif /* NET_CONNECTOR_HPP */
