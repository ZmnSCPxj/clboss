Here is our desired interface:

    auto db = Sqlite3::Db("filename");

    return Ev::lift().then([db]() {
        /* This could block if a transaction is currently in-flight.  */
        return db.transact();
    }).then([](Sqlite3::Tx tx) {
        auto res = tx.query("SELECT (a, b) FROM tablename WHERE id=:ID")
                 .bind(":ID", 42)
                 .execute()
                 ;
        /* `res` is a single-pass sequence!
         * It has an iterator type that is an input iterator
         * (can only dereference and advance, but copying
         * does not create a distinct state).
         */
        for (auto r : res) {
            auto a = r.column<int>(0);
            auto b = r.column<std::string>(1);
            process(a, b);
        }
        /* Sqlite3::Tx cannot be copied, but can be moved.
         * Once it has been destructed, it automatically
         * rolls back and lets other greenthreads execute
         * their transactions.
         *
         * In order to actually commit the transaction,
         * you have to call Sqlite3::Tx::commit on it.
         * This atomically writes the changes to disk,
         * destroys the Tx object (meaning it is in an
         * invalid, moved-from state where it cannot be
         * used for queries) and lets other greenthreads
         * waiting to transact continue.
         */
        tx.commit();
        
        return Ev::lift();
    });


`Sqlite3::Tx::rollback` can be used to explicitly rollback a transaction,
in which case you can no longer call `Sqlite3::Tx::query` on it.

