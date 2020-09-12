#ifndef SQLITE3_DB_HPP
#define SQLITE3_DB_HPP

#include<memory>
#include<string>

namespace Ev { template<typename a> class Io; }
namespace Sqlite3 { class Result; }
namespace Sqlite3 { class Tx; }

namespace Sqlite3 {

/** class Sqlite3::Db
 *
 * @brief class to access an SQLITE3 database.
 *
 * @desc The primary member function here is `transact`.
 * This will return an action that, when it returns,
 * provides a `Sqlite3::Tx` object.
 */
class Db {
private:
	class Impl;
	std::shared_ptr<Impl> pimpl;

	friend class Sqlite3::Result;
	friend class Sqlite3::Tx;

	void* get_connection() const;
	void transaction_finish();

public:
	/* Opens a database, creating it
	 * if absent.
	 * As typical for SQLITE3, ":memory:" creates an in-memory
	 * db, "" creates a new temporary db.
	 */
	explicit
	Db(std::string const& filename);

	/* Creates an empty/invalid db object.  */
	Db() =default;
	Db(Db const&) =default;
	Db(Db&&) =default;
	Db& operator=(Db const&) =default;
	Db& operator=(Db&&) =default;
	~Db() =default;

	/* If the Db is false (invalid), you cannot use transact().  */
	operator bool() const { return !!pimpl; }
	bool operator!() const { return !pimpl; }

	/** Sqlite3::Db::transact
	 *
	 * @desc Creates a database transaction, ensuring
	 * exclusive access by blocking if an `Sqlite3::Tx`
	 * is still in-flight.
	 */
	Ev::Io<Sqlite3::Tx> transact();
};

}

#endif /* !defined(SQLITE3_DB_HPP) */
