Here is our desired interface:

    auto db = Sqlite3::Db("filename");

    return Ev::lift().then([db]() {
        /* This could block if a transaction is currently in-flight.  */
        return db.transact();
    }).then([](Sqlite3::Tx tx) {
        auto it = tx.query("SELECT (a, b) FROM tablename WHERE id=:ID")
                .bind(":ID", 42)
                .execute()
                ;
        /* `it` is a single-pass input iterator.  */
        for (; it != tx.end(); ++it) {
            auto a = it->column<int>(0);
            auto b = it->column<std::string>(1);
            process(a, b);
        }
        /* Sqlite3::Tx cannot be copied, but can be moved.
         * Once it has been destructed, it automatically
         * commits and lets other greenthreads execute
         * their transactions.
         */
        return Ev::lift();
    });


`Sqlite3::Tx::rollback` can be used to rollback a transaction,
in which case you can no longer call `Sqlite3::Tx::query` on it.

