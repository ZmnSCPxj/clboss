#ifndef LN_PREIMAGE_HPP
#define LN_PREIMAGE_HPP

#include<cstdint>
#include<memory>
#include<string>

namespace Ripemd160 { class Hash; }
namespace Secp256k1 { class Random; }
namespace Sha256 { class Hash; }

namespace Ln {

/** class Ln::Preimage
 *
 * @brief a preimage for a hash, which can
 * be used with the LN.
 */
class Preimage {
private:
	struct Impl {
		std::uint8_t data[32];
	};
	std::shared_ptr<Impl> pimpl;

public:
	Preimage() =default;
	Preimage(Preimage const&) =default;
	Preimage(Preimage&&) =default;
	Preimage& operator=(Preimage const&) =default;
	Preimage& operator=(Preimage&&) =default;
	~Preimage() =default;

	Preimage(Secp256k1::Random&);

	static
	bool valid_string(std::string const&);
	Preimage(std::string const&);

	explicit
	operator std::string() const;

	bool operator==(Preimage const&) const;
	bool operator!=(Preimage const& o) const {
		return !(*this == o);
	}

	explicit
	operator bool() const;
	bool operator!() const {
		return !bool(*this);
	}

	void to_buffer(std::uint8_t data[32]) const {
		if (pimpl)
			for (auto i = std::size_t(0); i < 32; ++i)
				data[i] = pimpl->data[i];
		else
			for (auto i = std::size_t(0); i < 32; ++i)
				data[i] = 0;
	}
	void from_buffer(std::uint8_t const data[32]) {
		if (!pimpl)
			pimpl = std::make_shared<Impl>();
		for (auto i = std::size_t(0); i < 32; ++i)
			pimpl->data[i] = data[i];
	}

	Sha256::Hash sha256() const;
	Ripemd160::Hash hash160() const;
};

}

#endif /* !defined(LN_PREIMAGE_HPP) */
