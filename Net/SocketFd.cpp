#include<errno.h>
#include<stdexcept>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>
#include"Net/SocketFd.hpp"

namespace Net {

SocketFd::~SocketFd() {
	if (!fd)
		return;

	/* Save the errno; it is possible we are shutting down FDs
	 * due to failures, so make sure the reported failure is
	 * the one that is saved in the current errno.
	 */
	auto my_errno = errno;

	char unused[64];
	auto sys_res = int();

	/* Tell remote side that we will stop sending data and
	 * trigger an EOF there.
	 */
	sys_res = shutdown(fd.get(), SHUT_WR);
	if (sys_res < 0) {
		/* Restore the previous errno.  */
		errno = my_errno;
		/* Normal ~Fd() will close the socket file descriptor.  */
		return;
	}

	/* Drain the incoming socket until EOF.  */
	for (;;) {
		do {
			sys_res = ::read(fd.get(), unused, sizeof(unused));
		} while (sys_res < 0 && errno == EINTR);
		/* Let normal ~Fd() close.
		 * sys_res is 0 at EOF, and -1 at error.
		 */
		if (sys_res <= 0) {
			/* Restore the previous errno.  */
			errno = my_errno;
			return;
		}
	}
}

void SocketFd::write(std::vector<std::uint8_t> const& data) {
	auto size = data.size();
	auto ptr = &data[0];
	do {
		auto res = ssize_t();
		do {
			res = ::write( fd.get()
				     , ptr, size
				     );
		} while (res < 0 && errno == EINTR);
		if (res < 0)
			throw std::runtime_error( std::string("Net::SocketFd::write: write: ")
						+ strerror(errno)
						);
		ptr += res;
		size -= res;
	} while (size > 0);
}

std::vector<std::uint8_t> SocketFd::read(std::size_t size) {
	auto ret = std::vector<std::uint8_t>(size);
	auto ptr = &ret[0];
	do {
		auto res = ssize_t();
		do {
			res = ::read( fd.get()
				    , ptr, size
				    );
		} while (res < 0 && errno == EINTR);
		if (res < 0)
			throw std::runtime_error( std::string("Net::SocketFd::read: read: ")
						+ strerror(errno)
						);
		if (res == 0) {
			ret.resize(ptr - &ret[0]);
			return ret;
		}
		ptr += res;
		size -= res;
	} while (size > 0);
	return ret;
}

}

