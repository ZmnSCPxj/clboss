#include<errno.h>
#include<unistd.h>
#include"Util/Rw.hpp"

namespace Util { namespace Rw {

bool write_all(int fd, void const* p, std::size_t s) {
	do  {
		auto res = ssize_t();
		do {
			res = write(fd, p, s);
		} while (res < 0 && ( errno == EINTR
				   || errno == EWOULDBLOCK
				   || errno == EAGAIN
				    ));
		if (res < 0)
			return false;

		s -= size_t(res);
		p = (void*)(((char*) p) + res);
	} while (s > 0);
	return true;
}

bool read_all(int fd, void* p, std::size_t& size) {
	auto s = size;
	size = 0;
	do {
		auto res = ssize_t();
		do {
			res = read(fd, p, s);
		} while (res < 0 && ( errno == EINTR
				   || errno == EWOULDBLOCK
				   || errno == EAGAIN
				    ));
		if (res < 0)
			return false;
		if (res == 0)
			/* EOF.  */
			return false;

		size += size_t(res);
		s -= size_t(res);
		p = (void*)(((char*) p) + res);
	} while (s > 0);
	return true;
}

}}
