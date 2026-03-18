#include "../database.h"
#include <iostream>

bool Database::prepareSessionRepository()
{
        const char *createSessionTableSQL =
            "CREATE TABLE IF NOT EXISTS sessions ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "userId INT NOT NULL,"
            "accessTokenHash BLOB NOT NULL,"
            "refreshTokenHash TEXT NOT NULL,"
            "accessExpires INT,"
            "refreshExpires INT"
            ");";
        int rc = sqlite3_exec(db, createSessionTableSQL, nullptr, nullptr, nullptr);
        if (!check(rc, "sql create session table error"))
                return false;
        if (!prepare(insert_session_stmt,
                     "INSERT INTO sessions(userId, accessTokenHash, refreshTokenHash, accessExpires, refreshExpires) VALUES (?1 , ?2 , ?3 , "
                     "strftime('%s', 'now', '+45 minutes'),"
                     "strftime('%s', 'now', '+60 days')"
                     ");",
                     "sql insert session statement prepare error"))
                return false;
        if (!prepare(check_session_exist_stmt,
                     "SELECT * FROM sessions WHERE accessTokenHash = ?1 OR refreshTokenHash = ?2 LIMIT 1;",
                     "sql check_session_exist_statement prepare error"))
                return false;

        if (!prepare(get_session_from_id_stmt,
                     "SELECT * FROM sessions WHERE id = ?1 ;",
                     "sql get_session_access_token_statement prepare error"))
                return false;
        return true;
}

bool Database::insertSession(int userId, std::string accessTokenHash, const char *refreshTokenHash)
{
        if (!valid)
                return false;
        auto fail = [&](int rc, int stepIndex)
        {
                std::cerr << "SQLite error (" << rc << ") on insertSession , step " << stepIndex << ": " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(insert_session_stmt);
                return false;
        };

        int rc = sqlite3_bind_int(insert_session_stmt, 1, userId);
        if (rc != SQLITE_OK)
                return fail(rc, 1);
        rc = sqlite3_bind_blob(insert_session_stmt, 2, accessTokenHash.data(), accessTokenHash.size(), SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
                return fail(rc, 2);
        rc = sqlite3_bind_text(insert_session_stmt, 3, refreshTokenHash, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
                return fail(rc, 3);

        rc = sqlite3_step(insert_session_stmt);
        if (rc != SQLITE_DONE)
                return fail(rc, 4);

        cleanup_stmt(insert_session_stmt);
        return true;
}

bool Database::sessionExists(std::string accessTokenHash, const char *refreshTokenHash, bool &result)
{
        if (!valid)
                return false;
        auto fail = [&](int rc, int stepIndex)
        {
                std::cerr << "SQLite error (" << rc << ") on sessionExists , step " << stepIndex << ": " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(check_session_exist_stmt);
                return false;
        };

        int rc = sqlite3_bind_blob(check_session_exist_stmt, 1, accessTokenHash.data(), accessTokenHash.size(), SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
                return fail(rc, 1);
        rc = sqlite3_bind_text(check_session_exist_stmt, 2, refreshTokenHash, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
                return fail(rc, 2);

        rc = sqlite3_step(check_session_exist_stmt);
        if (rc == SQLITE_ROW)
        {
                result = true;
        }
        else if (rc == SQLITE_DONE)
        {
                result = false;
        }
        else
        {
                std::cerr << "SQLite error (" << rc << ") on Database::sessionExists sqlite3_step : " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(check_session_exist_stmt);
                return false;
        }
        cleanup_stmt(check_session_exist_stmt);
        return true;
}

bool Database::getSessionFromId(int sessionId, Session &session, bool &isFound)
{
        isFound = false;
        if (!valid)
                return false;
        int rc = sqlite3_bind_int(get_session_from_id_stmt, 1, sessionId);
        if (rc != SQLITE_OK)
                goto error;
        rc = sqlite3_step(get_session_from_id_stmt);
        if (rc == SQLITE_DONE)
        {
                isFound = false;
                cleanup_stmt(get_session_from_id_stmt);
                return true;
        }
        else if (rc == SQLITE_ROW)
        {
                if (sqlite3_column_type(get_session_from_id_stmt, 0) != SQLITE_INTEGER)
                        goto type_error;
                session.sessionId = sqlite3_column_int(get_session_from_id_stmt, 0);

                if (sqlite3_column_type(get_session_from_id_stmt, 1) != SQLITE_INTEGER)
                        goto type_error;
                session.userid = sqlite3_column_int(get_session_from_id_stmt, 1);

                if (sqlite3_column_type(get_session_from_id_stmt, 2) != SQLITE_BLOB)
                        goto type_error;
                int blobSize = sqlite3_column_bytes(get_session_from_id_stmt, 2);
                sqlite3_column_blob(get_session_from_id_stmt, 2);
                const char *blobPtr = reinterpret_cast<const char *>(sqlite3_column_blob(get_session_from_id_stmt, 2));
                if (!blobPtr)
                        goto error;
                session.accessTokenHash.assign(blobPtr, blobSize);

                if (sqlite3_column_type(get_session_from_id_stmt, 3) != SQLITE_TEXT)
                        goto type_error;
                int len = sqlite3_column_bytes(get_session_from_id_stmt, 3);
                const unsigned char *refreshTokenHashText = sqlite3_column_text(get_session_from_id_stmt, 3);
                if (!refreshTokenHashText)
                        goto error;
                session.refreshTokenHash.assign(reinterpret_cast<const char *>(refreshTokenHashText),len);

                if (sqlite3_column_type(get_session_from_id_stmt, 4) != SQLITE_INTEGER)
                        goto type_error;
                session.accessExpiry = sqlite3_column_int64(get_session_from_id_stmt, 4);

                if (sqlite3_column_type(get_session_from_id_stmt, 5) != SQLITE_INTEGER)
                        goto type_error;
                session.refreshExpiry = sqlite3_column_int64(get_session_from_id_stmt, 5);

                isFound = true;
                cleanup_stmt(get_session_from_id_stmt);
                return true;
        }
error:
        std::cerr << "SQLite error (" << rc << ") on Database::getSessiomFromId : " << sqlite3_errmsg(db) << std::endl;
        cleanup_stmt(get_session_from_id_stmt);
        return false;
type_error:
        std::cerr << "SQLite error on Database::getSessionFromId : incorrect type" << std::endl;
        cleanup_stmt(get_session_from_id_stmt);
        return false;
}