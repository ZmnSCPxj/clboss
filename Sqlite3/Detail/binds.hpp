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
struct Bind<std::int8_t> {
	static void bind(void *stmt, int l, std::int8_t v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<std::uint8_t> {
	static void bind(void *stmt, int l, std::uint8_t v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<std::int16_t> {
	static void bind(void *stmt, int l, std::int16_t v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<std::uint16_t> {
	static void bind(void *stmt, int l, std::uint16_t v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<std::int32_t> {
	static void bind(void *stmt, int l, std::int32_t v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<std::uint32_t> {
	static void bind(void *stmt, int l, std::uint32_t v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<std::int64_t> {
	static void bind(void *stmt, int l, std::int64_t v) {
		bind_i(stmt, l, v);
	}
};
template<>
struct Bind<std::uint64_t> {
	static void bind(void *stmt, int l, std::uint64_t v) {
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
