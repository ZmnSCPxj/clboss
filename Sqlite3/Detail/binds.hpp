#ifndef SQLITE3_DETAILS_BINDS_HPP
#define SQLITE3_DETAILS_BINDS_HPP

#include<cstddef>
#include<cstdint>
#include<string>

namespace Sqlite3 { namespace Detail {

void bind_d(void* stmt, int l, double);
void bind_i(void* stmt, int l, std::int64_t);
void bind_s(void* stmt, int l, std::string);
void bind_null(void *stmt, int l);

template<typename a>
struct Bind;

template<>
struct Bind<float> {
	static void bind(void *stmt, int l, float v) {
		bind_d(stmt, l, v);
	}
};
template<>
struct Bind<double> {
	static void bind(void *stmt, int l, double v) {
		bind_d(stmt, l, v);
	}
};

template<>
struct Bind<signed char> {
	static void bind(void *stmt, int l, signed char v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<unsigned char> {
	static void bind(void *stmt, int l, unsigned char v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<short> {
	static void bind(void *stmt, int l, short v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<unsigned short> {
	static void bind(void *stmt, int l, unsigned short v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<int> {
	static void bind(void *stmt, int l, int v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<unsigned int> {
	static void bind(void *stmt, int l, unsigned int v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<long> {
	static void bind(void *stmt, int l, long v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<unsigned long> {
	static void bind(void *stmt, int l, unsigned long v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<long long> {
	static void bind(void *stmt, int l, long long v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<unsigned long long> {
	static void bind(void *stmt, int l, unsigned long long v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<bool> {
	static void bind(void *stmt, int l, bool v) {
		bind_i(stmt, l, v ? 1 : 0);
	}
};

template<>
struct Bind<char const*> {
	static void bind(void *stmt, int l, char const* v) {
		bind_s(stmt, l, v);
	}
};
template<>
struct Bind<std::string> {
	static void bind(void *stmt, int l, std::string v) {
		bind_s(stmt, l, std::move(v));
	}
};

template<>
struct Bind<std::nullptr_t> {
	static void bind(void *stmt, int l, std::nullptr_t) {
		bind_null(stmt, l);
	}
};

}}

#endif /* !defined(SQLITE3_DETAILS_BINDS_HPP) */
