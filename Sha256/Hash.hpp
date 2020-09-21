#ifndef SHA256_HASH_HPP
#define SHA256_HASH_HPP

#include<cstdint>
#include<iostream>
#include<memory>
#include<string>
#include<utility>

namespace Sha256 { class Hasher; }

namespace Sha256 {

class Hash {
private:
	struct Impl {
		std::uint8_t d[32];
	};
	std::shared_ptr<Impl> pimpl;

	explicit
	Hash(std::uint8_t d[32]) {
		pimpl = std::make_shared<Impl>();
		for (auto i = std::size_t(0); i < 32; ++i)
			pimpl->d[i] = d[i];
	}

	friend class Sha256::Hasher;
	friend class std::hash<Hash>;

public:
	Hash() =default;
	Hash(Hash const&) =default;
	Hash(Hash&&) =default;
	Hash& operator=(Hash const&) =default;
	Hash& operator=(Hash&&) =default;
	~Hash() =default;

	static
	bool valid_string(std::string const&);
	explicit
	Hash(std::string const&);

	explicit
	operator std::string() const;

	explicit
	operator bool() const;
	bool operator!() const {
		return !bool(*this);
	}

	bool operator==(Hash const&) const;
	bool operator!=(Hash const& i) const {
		return !(*this == i);
	}

	void to_buffer(std::uint8_t d[32]) const {
		if (pimpl)
			for (auto i = std::size_t(0); i < 32; ++i)
				d[i] = pimpl->d[i];
		else
			for (auto i = std::size_t(0); i < 32; ++i)
				d[i] = 0;
	}
	void from_buffer(std::uint8_t const d[32]) {
		if (!pimpl)
			pimpl = std::make_shared<Impl>();
		for (auto i = std::size_t(0); i < 32; ++i)
			pimpl->d[i] = d[i];
	}
};

inline
std::ostream& operator<<(std::ostream& os, Hash const& i) {
	return os << std::string(i);
}
inline
std::istream& operator>>(std::istream& is, Hash& i) {
	auto s = std::string();
	is >> s;
	i = Hash(s);
	return is;
}

}

/* For use with std::unordered_map.  */
namespace std {
	template<>
	struct hash<::Sha256::Hash> {
		std::size_t operator()(::Sha256::Hash const& i) const {
			if (!i.pimpl)
				return 0;
			/* Already a hash, so just read it as-is.  */
			return *reinterpret_cast<std::size_t*>(i.pimpl->d);
		}
	};
}

#endif /* !defined(SHA256_HASH_HPP) */
