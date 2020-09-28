#ifndef BOSS_MSG_DBRESOURCE_HPP
#define BOSS_MSG_DBRESOURCE_HPP

#include"Sqlite3/Db.hpp"

namespace Boss { namespace Msg {

/** struct Boss::Msg::DbResource
 *
 * @brief provides access to the db.
 *
 * @desc emitted before `Boss::Msg::Init`.
 * The same database is broadcast in this and
 * in `Boss::Msg::Init`.
 *
 * However, this message exists to assist in
 * testing otherwise-logic-only modules that
 * have persistent data.
 * `Init` includes a lot of other items that
 * are a pain to mock in testing, so a module
 * that is interested only in the database
 * and on bus messages can listen to this
 * instead of `Init`.
 */
struct DbResource {
	Sqlite3::Db db;
};

}}

#endif /* !defined(BOSS_MSG_DBRESOURCE_HPP) */
