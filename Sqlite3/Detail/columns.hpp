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
struct Column<signed char> {
	static
	signed char column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<unsigned char> {
	static
	unsigned char column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<short> {
	static
	short column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<unsigned short> {
	static
	unsigned short column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<int> {
	static
	int column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<unsigned int> {
	static
	unsigned int column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<long> {
	static
	long column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<unsigned long> {
	static
	unsigned long column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<long long> {
	static
	long long column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<unsigned long long> {
	static
	unsigned long long column(void* stmt, int c) {
		return column_i(stmt, c);
	}
};
template<>
struct Column<bool> {
	static
	bool column(void* stmt, int c) {
		return column_i(stmt, c) != 0;
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
