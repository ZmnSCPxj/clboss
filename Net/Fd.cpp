#include"Net/Fd.hpp"
#include<errno.h>
#include<unistd.h>

namespace Net {

Fd::~Fd() {
	if (fd >= 0) {
		/* Ignore errors.  */
		auto err = errno;
		close(fd);
		errno = err;
	}
}

}
