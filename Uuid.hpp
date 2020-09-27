#ifndef UUID_HPP
#define UUID_HPP

#include<cstdint>
#include<iostream>
#include<memory>
#include<string>
#include<utility>

/** class Uuid
 *
 * @brief a unique 128-bit identifier.
 *
 * @desc this is ***not*** an RFC4122-standard UUID!
 * This is just 16 random bytes that we think is
 * sufficiently unique for this application.
 */
class Uuid {
private:
	struct Impl {
		std::uint8_t data[16];
	};
	std::shared_ptr<Impl> pimpl;

public:
	Uuid() =default;
	Uuid(Uuid&&) =default;
	Uuid(Uuid const&) =default;
	Uuid& operator=(Uuid&&) =default;
	Uuid& operator=(Uuid const&) =default;
	~Uuid() =default;

	/* Construct a fresh random UUID that has never
	 * been used before (with high probability).  */
	static Uuid random();

	bool operator==(Uuid const& o) const;
	bool operator!=(Uuid const& o) const {
		return !(*this == o);
	}

	operator bool() const;
	bool operator!() const {
		return !bool(*this);
	}

	explicit operator std::string() const;
	explicit Uuid(std::string const&);
	static
	bool valid_string(std::string const&);

	std::size_t hash() const {
		if (!pimpl)
			return 0;
		return *reinterpret_cast<std::size_t*>(pimpl->data);
	}
};

inline
std::ostream& operator<<(std::ostream& os, Uuid const& i) {
	return os << std::string(i);
}
inline
std::istream& operator>>(std::istream& is, Uuid& i) {
	auto s = std::string();
	is >> s;
	i = Uuid(s);
	return is;
}

namespace std {

template<>
struct hash<Uuid> {
	std::size_t operator()(Uuid const& i) const {
		return i.hash();
	}
};

}

#endif /* !defined(UUID_HPP) */
