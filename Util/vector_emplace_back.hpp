#ifndef UTIL_VECTOR_EMPLACE_BACK_HPP
#define UTIL_VECTOR_EMPLACE_BACK_HPP

#include<utility>
#include<vector>

namespace Util {

namespace VectorEmplaceBackPrivate {

template<typename T>
struct Emplacer {
	::std::vector<T>& vec;

	Emplacer(::std::vector<T>& vec_) : vec(vec_) { }

	template<typename... As>
	void emplace_back(As&&... as) {
		vec.emplace_back(::std::forward<As>(as)...);
	}
};
template<>
struct Emplacer<bool> {
	::std::vector<bool>& vec;

	Emplacer(::std::vector<bool>& vec_) : vec(vec_) { }

	template<typename... As>
	void emplace_back(As&&... as) {
		vec.push_back(bool(::std::forward<As>(as)...));
	}
};

}

/** Util::vector_emplace_back
 *
 * @desc Templated function to emplace an object into a vector of
 * specific types.
 *
 * C++11 defines `std::vector<bool>` specially, indicating that the
 * it may use a "packed" form where 8 `bool` objects are stored in the
 * same space as a `char`.
 *
 * However, in C++11 some operations on `std::vector<T>` are **not**
 * present in `std::vector<bool>`, precisely because of the possibility
 * of using a "packed" form for `std::vector<bool>` objects.
 * `emplace_back` is one of those functions.
 *
 * This limitation is lifted in C++14, however some compilers may be
 * strict in imposing this limitation for `std::vector<bool>`.
 */
template<typename T, typename... As>
void vector_emplace_back(std::vector<T>& vec, As&&... as) {
	auto emplacer = VectorEmplaceBackPrivate::Emplacer<T>(vec);
	emplacer.emplace_back(::std::forward<As>(as)...);
}

}

#endif /* !defined(UTIL_VECTOR_EMPLACE_BACK_HPP) */
