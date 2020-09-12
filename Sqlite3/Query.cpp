#include"Sqlite3/Db.hpp"
#include"Sqlite3/Query.hpp"
#include"Sqlite3/Result.hpp"
#include"Util/make_unique.hpp"
#include<sqlite3.h>
#include<stdexcept>

namespace Sqlite3 {

class Query::Impl {
private:
	Db db;
	sqlite3_stmt* stmt;

public:
	Impl( Db const& db_
	    , void* stmt_
	    ) : db(db_)
	      , stmt((sqlite3_stmt*) stmt_)
	      { }
	~Impl() {
		if (stmt)
			(void) sqlite3_finalize(stmt);
	}

	void* get_stmt() const { return stmt; }
	int get_location(char const* field) const {
		auto res = sqlite3_bind_parameter_index(stmt, field);
		if (res == 0)
			throw std::runtime_error(
				std::string("Sqlite3::Query::bind: "
					    "no field: "
					   ) + field
			);
		return res;
	}

	Result execute() {
		/* Move responsibility off ourself.  */
		auto my_stmt = stmt;
		stmt = nullptr;
		/* Result constructor executes first sqlite3_step.  */
		return Result(db, my_stmt);
	}
};

Query::Query(Sqlite3::Db const& db, void* stmt)
	: pimpl(Util::make_unique<Impl>(db, stmt)) { }
Query::Query(Query&& o) : pimpl(std::move(o.pimpl)) { }
Query::~Query() { }

void* Query::get_stmt() const { return pimpl->get_stmt(); }
int Query::get_location(const char* field) const {
	return pimpl->get_location(field);
}

Result Query::execute() {
	auto r = pimpl->execute();
	pimpl = nullptr;
	return r;
}

}
