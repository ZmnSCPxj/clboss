#ifndef UTIL_STRINGIFY_HPP
#define UTIL_STRINGIFY_HPP

#include<sstream>
#include<string>

namespace Util {

template<typename a>
std::string stringify(a const& v) {
	auto os = std::ostringstream();
	os << v;
	return os. str();
}

}

#endif /* !defined(UTIL_STRINGIFY_HPP) */
