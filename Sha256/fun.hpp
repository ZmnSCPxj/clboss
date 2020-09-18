#ifndef SHA256_FUN_HPP
#define SHA256_FUN_HPP

#include"Sha256/Hash.hpp"
#include"Sha256/Hasher.hpp"

namespace Sha256 {

inline
Sha256::Hash fun(void const* p, std::size_t len) {
	auto hasher = Sha256::Hasher();
	hasher.feed(p, len);
	return std::move(hasher).finalize();
}

}

#endif /* !defined(SHA256_FUN_HPP) */
