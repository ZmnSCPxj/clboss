#ifndef SQLITE3_QUERY_HPP
#define SQLITE3_QUERY_HPP

#include"Sqlite3/Detail/binds.hpp"
#include<memory>
#include<string>

namespace Sqlite3 { class Db; }
namespace Sqlite3 { class Result; }
namespace Sqlite3 { class Tx; }

namespace Sqlite3 {

/** class Sqlite3::Query
 *
 * @brief Represents a query that will have parameters bound,
 * then later executed to produce a result.
 */
class Query {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

	friend class Sqlite3::Tx;
	Query(Sqlite3::Db const&, void*);

	void* get_stmt() const;
	int get_location(char const*) const;

public:
	Query() =delete;
	Query(Query&&);
	~Query();

	/** Sqlite3::Query::bind
	 *
	 * @brief binds a named parameter of the query.
	 * Named parameters are of the form :VVV, @VVV,
	 * or $VVV, where VVV is alphanumeric.
	 */
	template<typename a>
	Query& bind(char const* field, a value) {
		Detail::Bind<a>::bind( get_stmt(), get_location(field)
				     , std::move(value)
				     );
		return *this;
	}
	template<typename a>
	Query& bind(std::string const& field, a value) {
		return bind<a>(field.c_str(), value);
	}

	/** Sqlite3::Query::execute
	 *
	 * @brief executes the query.
	 * Pre-condition: you should have bound all the
	 * parameters (or else they would be NULL).
	 * Post-condition: the query cannot be reused
	 * and is in an "invalid" state.
	 */
	Result execute();
};

}

#endif /* !defined(SQLITE3_QUERY_HPP) */
