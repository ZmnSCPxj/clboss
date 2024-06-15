#ifndef UTIL_STREAM_ELEMENTS_HPP
#define UTIL_STREAM_ELEMENTS_HPP

#include<sstream>
#include<string>

namespace Util {

template<typename Container>
std::ostream& stream_elements(std::ostream& os, Container const& v) {
    os << "[";
    auto it = std::begin(v);
    auto end = std::end(v);
    if (it != end) {
        os << *it;
        ++it;
    }
    for (; it != end; ++it) {
        os << ", " << *it;
    }
    os << "]";
    return os;
}

}

#endif /* !defined(UTIL_STREAM_ELEMENTS_HPP) */
