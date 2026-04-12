#include <iostream>
#include "../database.h"

bool Database::prepareMessagesRepository()
{
        const char *createMessagesTableSQL =
            "CREATE TABLE IF NOT EXISTS messages ("
            "senderUserId INT NOT NULL,"
            "receiverSessionId INT NOT NULL,"
            "message BLOB NOT NULL,"
            "timestamp INT NOT NULL"
            ");";
        int rc = sqlite3_exec(db, createMessagesTableSQL, nullptr, nullptr, nullptr);
        if (!check(rc, "sql create messages table error"))
                return false;
        if (!prepare(insert_message_stmt,
                     "INSERT INTO messages(senderUserId, receiverSessionId, message, timestamp) VALUES (?1 , ?2 , ?3 , ?4);",
                     "sql insert message statement prepare error"))
                return false;
        return true;
}

bool Database::insertMessage(userIdType senderUserId, sessionIdType receiverSessionId, std::string& message, int64_t timestamp)
{
        if (!valid)
                return false;
        auto fail = [&](int rc, int stepIndex)
        {
                std::cerr << "SQLite error (" << rc << ") on insertMessage , step " << stepIndex << ": " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(insert_message_stmt);
                return false;
        };

        int rc = sqlite3_bind_int64(insert_message_stmt, 1, senderUserId);
        if (rc != SQLITE_OK)
                return fail(rc, 1);
        rc = sqlite3_bind_int64(insert_message_stmt, 2, receiverSessionId);
        if (rc != SQLITE_OK)
                return fail(rc, 2);
        rc = sqlite3_bind_blob64(insert_message_stmt, 3, message.data(),message.size(), SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
                return fail(rc, 3);
        rc = sqlite3_bind_int64(insert_message_stmt, 4, timestamp);
        if (rc != SQLITE_OK)
                return fail(rc, 4);
        rc = sqlite3_step(insert_message_stmt);
        if (rc != SQLITE_DONE)
                return fail(rc, 5);
        cleanup_stmt(insert_message_stmt);
        return true;
}