#include"Util/BacktraceException.hpp"
#include"Sqlite3/Detail/binds.hpp"
#include<sqlite3.h>
#include<stdexcept>

namespace Sqlite3 { namespace Detail {

void bind_d(void* stmt, int l, double v) {
	auto res = sqlite3_bind_double( (sqlite3_stmt*) stmt
				      , l, v
				      );
	if (res != SQLITE_OK)
		throw Util::BacktraceException<std::runtime_error>("Sqlite3: bind error.");
}
void bind_i(void* stmt, int l, std::int64_t v) {
	auto res = sqlite3_bind_int64( (sqlite3_stmt*) stmt
				     , l, v
				     );
	if (res != SQLITE_OK)
		throw Util::BacktraceException<std::runtime_error>("Sqlite3: bind error.");
}
void bind_s(void* stmt, int l, std::string v) {
	auto res = sqlite3_bind_text( (sqlite3_stmt*) stmt
				    , l, v.c_str(), v.size()
				    , SQLITE_TRANSIENT
				    );
	if (res != SQLITE_OK)
		throw Util::BacktraceException<std::runtime_error>("Sqlite3: bind error.");
}
void bind_null(void* stmt, int l) {
	auto res = sqlite3_bind_null( (sqlite3_stmt*) stmt, l);
	if (res != SQLITE_OK)
		throw Util::BacktraceException<std::runtime_error>("Sqlite3: bind error.");
}

}}
