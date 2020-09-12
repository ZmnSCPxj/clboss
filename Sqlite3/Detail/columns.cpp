#include"Sqlite3/Detail/columns.hpp"
#include<algorithm>
#include<sqlite3.h>

namespace Sqlite3 { namespace Detail {

double column_d(void* stmt, int c) {
	return sqlite3_column_double((sqlite3_stmt*) stmt, c);
}
std::int64_t column_i(void* stmt, int c) {
	return sqlite3_column_int64((sqlite3_stmt*) stmt, c);
}
std::string column_s(void* vstmt, int c) {
	auto stmt = (sqlite3_stmt*) vstmt;
	auto len = sqlite3_column_bytes(stmt, c);
	auto dat = sqlite3_column_text(stmt, c);

	auto s = std::string();
	s.resize(len);
	std::copy(dat, dat + len, s.begin());

	return s;
}

}}
