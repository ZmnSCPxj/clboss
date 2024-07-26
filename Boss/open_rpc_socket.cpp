#include"Boss/open_rpc_socket.hpp"
#include"Net/Fd.hpp"
#include"Util/BacktraceException.hpp"
#include<errno.h>
#include<stdexcept>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include<unistd.h>

namespace Boss {

Net::Fd open_rpc_socket( std::string const& lightning_dir
		       , std::string const& rpc_file
		       ) {
	auto chdir_res = chdir(lightning_dir.c_str());
	if (chdir_res < 0)
		throw Util::BacktraceException<std::runtime_error>(
			std::string("chdir: ") + strerror(errno)
		);

	auto fd = Net::Fd(socket(AF_UNIX, SOCK_STREAM, 0));
	if (!fd)
		throw Util::BacktraceException<std::runtime_error>(
			std::string("socket: ") + strerror(errno)
		);

	auto addr = sockaddr_un();
	if (rpc_file.length() + 1 > sizeof(addr.sun_path))
		throw Util::BacktraceException<std::runtime_error>(
			std::string("sizeof(sun_path): ") + strerror(ENOSPC)
		);

	strcpy(addr.sun_path, rpc_file.c_str());
	addr.sun_family = AF_UNIX;

	auto connect_res = int();
	do {
		connect_res = connect( fd.get()
				     , reinterpret_cast<sockaddr const*>(&addr)
				     , sizeof(addr)
				     );
	} while (connect_res < 0 && errno == EINTR);
	if (connect_res < 0)
		throw Util::BacktraceException<std::runtime_error>(
			std::string("connect: ") + strerror(errno)
		);

	return fd;
}

}
