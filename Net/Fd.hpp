#ifndef NET_FD_HPP
#define NET_FD_HPP

#include<cstddef>
#include<memory>

namespace Net {

/** class Net::Fd
 *
 * @brief RAII class for file descriptor.
 * This class is appropriate for local connections, and
 * is not appropriate (may lead to final writes not being
 * received by the other end) for remote.
 */
class Fd {
private:
	int fd;

public:
	Fd(std::nullptr_t _ = nullptr) : fd(-1) { }
	~Fd();

	explicit Fd(int fd_) : fd(fd_) { }
	/* Not copyable!  */
	Fd(Fd const&) =delete;
	/* Moveable.  */
	Fd(Fd&& o) : fd(o.release()) { }

	Fd& operator=(Fd&& o) {
		auto tmp = Fd(std::move(o));
		swap(tmp);
		return *this;
	}

	int get() const { return fd; }
	int release() {
		auto ret = fd;
		fd = -1;
		return ret;
	}
	void swap(Fd& o) {
		auto tmp = o.fd;
		o.fd = fd;
		fd = tmp;
	}
	void reset(int fd_ = -1) {
		auto tmp = Fd(fd_);
		swap(tmp);
	}

	explicit
	operator bool() const {
		return (fd >= 0);
	}
	bool operator!() const {
		return !((bool) *this);
	}
};

}

#endif /* !defined(NET_FD_HPP) */
