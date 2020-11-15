#ifndef NET_PROXYCONNECTOR_HPP
#define NET_PROXYCONNECTOR_HPP

#include<memory>
#include<utility>
#include"Net/Connector.hpp"

namespace Net {

class ProxyConnector : public Connector {
private:
	std::unique_ptr<Net::Connector> base;
	std::string proxy_host;
	int proxy_port;

	Net::SocketFd
	single_connect( std::string const& host, int port
		      , bool& socks5_general_error
		      );

public:
	ProxyConnector() =delete;
	explicit ProxyConnector( std::unique_ptr<Net::Connector> base_
			       , std::string proxy_host_
			       , int proxy_port_
			       ) : base(std::move(base_))
				 , proxy_host(std::move(proxy_host_))
				 , proxy_port(proxy_port_)
				 { }

	Net::SocketFd
	connect(std::string const& host, int port) override;
};

}

#endif /* NET_PROXYCONNECTOR_HPP */
