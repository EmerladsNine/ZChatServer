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
        if (!prepare(get_messages_for_session_stmt,
                     "SELECT senderUserId, receiverSessionId, message, timestamp FROM messages "
                     "WHERE receiverSessionId = ?1 ORDER BY timestamp ASC;",
                     "sql get messages for session statement prepare error"))
                return false;
        if (!prepare(delete_messages_before_timestamp_stmt,
                     "DELETE FROM messages WHERE receiverSessionId = ?1 AND timestamp <= ?2;",
                     "sql delete messages before timestamp statement prepare error"))
                return false;
        return true;
}

bool Database::insertMessage(userIdType senderUserId, sessionIdType receiverSessionId, std::string &message, int64_t timestamp)
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
        rc = sqlite3_bind_blob64(insert_message_stmt, 3, message.data(), message.size(), SQLITE_TRANSIENT);
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

bool Database::getMessagesForSession(sessionIdType receiverSessionId, std::vector<Message> &messagesOut)
{
        if (!valid)
                return false;

        int rc = sqlite3_bind_int64(get_messages_for_session_stmt, 1, receiverSessionId);
        if (rc != SQLITE_OK)
        {
                std::cerr << "SQLite error (" << rc << ") on Database::getMessagesForSession binding: " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(get_messages_for_session_stmt);
                return false;
        }

        return getMessagesInternal(get_messages_for_session_stmt, messagesOut);
}

bool Database::getMessagesInternal(sqlite3_stmt *readyToRunStmt, std::vector<Message> &messagesOut)
{
        int rc;
        while ((rc = sqlite3_step(readyToRunStmt)) == SQLITE_ROW)
        {
                Message msg;
                if (sqlite3_column_type(readyToRunStmt, 0) != SQLITE_INTEGER)
                        goto type_error;
                msg.senderUserId = sqlite3_column_int64(readyToRunStmt, 0);

                if (sqlite3_column_type(readyToRunStmt, 1) != SQLITE_INTEGER)
                        goto type_error;
                msg.receiverSessionId = sqlite3_column_int64(readyToRunStmt, 1);

                if (sqlite3_column_type(readyToRunStmt, 2) == SQLITE_BLOB)
                {
                        const void *blob = sqlite3_column_blob(readyToRunStmt, 2);
                        int bytes = sqlite3_column_bytes(readyToRunStmt, 2);

                        if (blob && bytes > 0)
                        {
                                const char *data = reinterpret_cast<const char *>(blob);
                                msg.message.assign(data, data + bytes);
                        }
                        else
                        {
                                msg.message.clear();
                        }
                }
                else
                {
                        goto type_error;
                }
                if (sqlite3_column_type(readyToRunStmt, 3) != SQLITE_INTEGER)
                        goto type_error;
                msg.timestamp = sqlite3_column_int64(readyToRunStmt, 3);

                messagesOut.push_back(std::move(msg));
        }

        if (rc != SQLITE_DONE)
        {
                std::cerr << "SQLite error (" << rc << ") on Database::getMessagesInternal step: " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(readyToRunStmt);
                return false;
        }

        cleanup_stmt(readyToRunStmt);
        return true;

type_error:
        std::cerr << "SQLite error on Database::getMessagesInternal : incorrect type" << std::endl;
        cleanup_stmt(readyToRunStmt);
        return false;
}

bool Database::deleteMessagesBefore(sessionIdType receiverSessionId, int64_t timestamp)
{
        if (!valid)
                return false;

        auto fail = [&](int rc, int stepIndex)
        {
                std::cerr << "SQLite error (" << rc << ") on deleteMessagesBefore , step " << stepIndex << ": " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(delete_messages_before_timestamp_stmt);
                return false;
        };

        // Bind Session ID
        int rc = sqlite3_bind_int64(delete_messages_before_timestamp_stmt, 1, receiverSessionId);
        if (rc != SQLITE_OK)
                return fail(rc, 1);

        // Bind Timestamp
        rc = sqlite3_bind_int64(delete_messages_before_timestamp_stmt, 2, timestamp);
        if (rc != SQLITE_OK)
                return fail(rc, 2);

        // Execute
        rc = sqlite3_step(delete_messages_before_timestamp_stmt);
        if (rc != SQLITE_DONE)
                return fail(rc, 3);

        cleanup_stmt(delete_messages_before_timestamp_stmt);
        return true;
}