#ifndef SQLITE3_RESULT_HPP
#define SQLITE3_RESULT_HPP

#include"Sqlite3/Db.hpp"
#include"Sqlite3/Detail/columns.hpp"
#include<iterator>

namespace Sqlite3 { class Query; }
namespace Sqlite3 { class Result; }
namespace Sqlite3 { class Row; }

namespace Sqlite3 {

/** class Sqlite3::Row
 *
 * @brief repersents a row in the results.
 */
class Row {
private:
	Result* r;

	friend class Sqlite3::Result;
	explicit
	Row(Result& r_) : r(&r_) { }

	Row() : r(nullptr) { }

public:
	Row(Row const&) =delete;
	Row(Row&) =delete;

	~Row() =default;

	template<typename a>
	a get(int c);
};

/** Sqlite3::Result
 *
 * @brief sequence of rows emitted by a query.
 *
 * @desc
 * IMPORTANT a `Sqlite3::Result` can only be traversed
 * exactly once!!!
 */
class Result {
private:
	Sqlite3::Db db;
	void* stmt;

	friend class Sqlite3::Query;
	friend class Sqlite3::Row;

	Result(Sqlite3::Db const& db_, void* stmt_);

	/* Return false at end of result.  */
	bool advance();

public:
	Result() =delete;
	Result(Result const&) =delete;
	Result(Result&&);
	~Result();

	Result& operator=(Result&& o) {
		auto tmp = std::move(o);
		std::swap(db, tmp.db);
		std::swap(stmt, tmp.stmt);
		return *this;
	}

	/** Sqlite3::Result::iterator
	 *
	 * @brief An input iterator that lets you traverse
	 * the results once.
	 */
	class iterator : Row {
	private:
		friend class Sqlite3::Result;
		explicit
		iterator(Result& r_) : Row(r_) {
			if (!r->stmt)
				r = nullptr;
		}

	public:
		iterator() : Row() { }
		iterator(iterator const& o) : Row() {
			r = o.r;
		}

		bool operator==(iterator const& o) const {
			return o.r == r;
		}
		bool operator!=(iterator const& o) const {
			return o.r != r;
		}

		iterator& operator++() {
			if (r)
				if (!r->advance())
					r = nullptr;
			return *this;
		}

		typedef std::input_iterator_tag iterator_category;
		typedef Row* pointer;
		typedef Row reference;
		typedef Row value_type;

		Row& operator*() {
			return *this;
		}
	};
	iterator begin() {
		return iterator(*this);
	}
	iterator end() {
		return iterator();
	}
};

template<typename a>
a Row::get(int c) {
	return Detail::Column<a>::column(r->stmt, c);
}

}

#endif /* !defined(SQLITE3_RESULT_HPP) */
