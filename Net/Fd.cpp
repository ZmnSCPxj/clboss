#include"Net/Fd.hpp"
#include<unistd.h>

namespace Net {

Fd::~Fd() {
	if (fd >= 0)
		close(fd);
}

}
