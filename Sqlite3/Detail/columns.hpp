#ifndef SQLITE3_DETAIL_COLUMNS_HPP
#define SQLITE3_DETAIL_COLUMNS_HPP

#include<cstdint>
#include<string>

namespace Sqlite3 { namespace Detail {

double column_d(void* stmt, int c);
std::int64_t column_i(void* stmt, int c);
std::string column_s(void* stmt, int c);

template<typename a>
struct Column;

template<>
struct Column<float> {
	static
	float column(void* stmt, int c) {
		return column_d(stmt, c);
	}
};
template<>
struct Column<double> {
	static
	double column(void* stmt, int c) {
		return column_d(stmt, c);
	}
};

template<>
struct Column<std::int8_t> {
	static
	std::int8_t column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<std::uint8_t> {
	static
	std::uint8_t column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<std::int16_t> {
	static
	std::int16_t column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<std::uint16_t> {
	static
	std::uint16_t column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<std::int32_t> {
	static
	std::int32_t column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<std::uint32_t> {
	static
	std::uint32_t column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<std::int64_t> {
	static
	std::int64_t column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<std::uint64_t> {
	static
	std::uint64_t column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};

template<>
struct Column<std::string> {
	static
	std::string column(void* stmt, int c) {
		return column_s(stmt, c);
	}
};

}}

#endif /* !defined(SQLITE3_DETAIL_COLUMNS_HPP) */
