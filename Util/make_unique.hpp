#ifndef UTIL_MAKE_UNIQUE_HPP
#define UTIL_MAKE_UNIQUE_HPP

#include<cstddef>
#include<memory>
#include<type_traits>
#include<utility>

namespace Util {

namespace MakeUniquePrivate {

template<typename T>
struct Trait { typedef ::std::unique_ptr<T> Single; };

template<typename T>
struct Trait<T[]> { typedef ::std::unique_ptr<T[]> Array; };

template<typename T, ::std::size_t N>
struct Trait<T[N]> { typedef void Bound; };

}

template<typename T, typename... As>
typename MakeUniquePrivate::Trait<T>::Single
make_unique(As&&... as) {
	return ::std::unique_ptr<T>(new T(::std::forward<As>(as)...));
}

template<typename T>
typename MakeUniquePrivate::Trait<T>::Array
make_unique(::std::size_t n) {
	typedef typename ::std::remove_extent<T>::type U;
	return ::std::unique_ptr<T>(new U[n]());
}

template<typename T, typename... As>
typename MakeUniquePrivate::Trait<T>::Bound
make_unique(As&&...) =delete;

}

#endif /* UTIL_MAKE_UNIQUE_HPP */
