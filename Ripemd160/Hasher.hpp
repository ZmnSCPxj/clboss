#ifndef RIPEMD160_HASHER_HPP
#define RIPEMD160_HASHER_HPP

#include<cstddef>
#include<memory>

namespace Ripemd160 { class Hash; }

namespace Ripemd160 {

/** class Ripemd160::Hasher
 *
 * @brief object that lets you stream bytes
 * into it, then generate the hash of all
 * the bytes streamed in.
 */
class Hasher {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Hasher();
	Hasher(Hasher&&);
	~Hasher();
	/* Duplicates midstate!  */
	Hasher(Hasher const&);

	Hasher& operator=(Hasher&&);
	Hasher& operator=(Hasher const&);

	explicit
	operator bool() const;
	bool operator!() const {
		return !bool(*this);
	}

	void feed(void const* p, std::size_t size);

	Ripemd160::Hash finalize()&&;
	Ripemd160::Hash get() const;
};

}

#endif /* !defined(RIPEMD160_HASHER_HPP) */
