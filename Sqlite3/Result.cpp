#include"Sqlite3/Db.hpp"
#include"Sqlite3/Result.hpp"
#include<sqlite3.h>

namespace Sqlite3 {

Result::Result(Sqlite3::Db const& db_, void* stmt_
	      ) : db(db_), stmt(stmt_) {
	advance();
}
Result::Result(Result&& o) {
	db = std::move(o.db);
	stmt = o.stmt;
	o.stmt = nullptr;
}
Result::~Result() {
	if (stmt)
		sqlite3_finalize((sqlite3_stmt*) stmt);
}

bool Result::advance() {
	auto ss = (sqlite3_stmt*) stmt;
	auto res = sqlite3_step(ss);
	if (res == SQLITE_DONE) {
		sqlite3_finalize((sqlite3_stmt*) stmt);
		stmt = nullptr;
		return false;
	} else if (res == SQLITE_ROW)
		return true;
	else {
		auto connection = (sqlite3*) db.get_connection();
		auto err = std::string(sqlite3_errmsg(connection));
		throw std::runtime_error(
			std::string("Sqlite3::Result: ") + err
		);
	}
}

}
