#ifndef SHA256_HASHER_HPP
#define SHA256_HASHER_HPP

#include<cstddef>
#include<memory>

namespace Sha256 { class Hash; }

namespace Sha256 {

/** class Sha256::Hasher
 *
 * @brief object that lets you stream bytes
 * to be fed to the hasher, then generate
 * the hash of all the bytes streamed in.
 */
class Hasher {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Hasher();
	Hasher(Hasher&&);
	~Hasher();
	/* Duplicates midstate.  */
	Hasher(Hasher const&);

	Hasher& operator=(Hasher&&);
	Hasher& operator=(Hasher const&);

	/* Is it still valid, or has it been finalized?  */
	explicit
	operator bool() const;
	bool operator!() const {
		return !bool(*this);
	}

	void feed(void const* p, std::size_t size);

	Sha256::Hash finalize()&&;
	/* Note: this is more expensive than finalize!
	 * Avoid using it if you can.
	 */
	Sha256::Hash get() const;
};

}

#endif /* !defined(SHA256_HASHER_HPP) */
