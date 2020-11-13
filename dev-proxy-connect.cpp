#include"Net/DirectConnector.hpp"
#include"Net/ProxyConnector.hpp"
#include"Net/SocketFd.hpp"
#include"Util/make_unique.hpp"
#include<errno.h>
#include<iostream>
#include<sstream>
#include<string.h>

namespace {

int parse_int(std::string s) {
	auto is = std::istringstream(std::move(s));
	auto rv = int();
	is >> rv;
	return rv;
}

}

int main(int argc, char** argv) {
	if (argc != 3 && argc != 5) {
		std::cerr << "Usage: dev-proxy-connect host port" << std::endl
			  << " OR:" << std::endl
			  << "Usage: dev-proxy-connect proxy proxyport host port" << std::endl
			  ;
		return 0;
	}

	auto conn = std::unique_ptr<Net::Connector>(
		Util::make_unique<Net::DirectConnector>()
	);

	auto host = std::string();
	auto port = int();
	if (argc == 3) {
		host = std::string(argv[1]);
		port = parse_int(argv[2]);
	} else if (argc == 5) {
		conn = Util::make_unique<Net::ProxyConnector>(
			std::move(conn),
			std::string(argv[1]),
			parse_int(argv[2])
		);
		host = std::string(argv[3]);
		port = parse_int(argv[4]);
	}

	auto fd = conn->connect(host, port);
	if (!fd) {
		auto err = std::string(strerror(errno));
		std::cout << "Not connected: " << err << " (" << errno << ")" << std::endl;
		return 1;
	}

	std::cout << "Connected." << std::endl;
	return 0;
}
