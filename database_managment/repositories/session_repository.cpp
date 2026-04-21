#include "../database.h"
#include <iostream>

bool Database::prepareSessionRepository()
{
        const char *createSessionTableSQL =
            "CREATE TABLE IF NOT EXISTS sessions ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "userId INT NOT NULL,"
            "accessTokenHash BLOB UNIQUE NOT NULL,"
            "refreshTokenHash TEXT UNIQUE NOT NULL,"
            "fcmToken TEXT UNIQUE,"
            "accessExpires INT,"
            "refreshExpires INT"
            ");";
        int rc = sqlite3_exec(db, createSessionTableSQL, nullptr, nullptr, nullptr);
        if (!check(rc, "sql create session table error"))
                return false;
        if (!prepare(insert_session_stmt,
                     "INSERT INTO sessions(userId, accessTokenHash, refreshTokenHash,fcmToken, accessExpires, refreshExpires) VALUES (?1 , ?2 , ?3 , null, "
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
        if (!prepare(update_session_stmt,
                     "UPDATE sessions SET accessTokenHash = ?1,"
                     "refreshTokenHash = ?2,"
                     "accessExpires = strftime('%s', 'now', '+45 minutes'), "
                     "refreshExpires = strftime('%s', 'now', '+60 days') "
                     "WHERE id = ?3;",
                     "sql update_session_statement prepare error"))
                return false;
        if (!prepare(get_sessions_from_user_id_stmt,
                     "SELECT * FROM sessions WHERE userId = ?1;",
                     "sql get_sessions_from_user_id_stmt prepare error"))
                return false;
        if (!prepare(update_fcm_token_stmt,
                     "UPDATE sessions SET fcmToken = ?1 WHERE id = ?2;",
                     "sql update_fcm_token_stmt prepare error"))
                return false;
        if (!prepare(remove_fcm_token_stmt,
                     "UPDATE sessions SET fcmToken = null WHERE fcmToken = ?1",
                     "sql remove_fcm_token_stmt prepare error"))
                return false;
        return true;
}

bool Database::insertSession(userIdType userId, std::string accessTokenHash, const char *refreshTokenHash)
{
        if (!valid)
                return false;
        auto fail = [&](int rc, int stepIndex)
        {
                std::cerr << "SQLite error (" << rc << ") on insertSession , step " << stepIndex << ": " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(insert_session_stmt);
                return false;
        };

        int rc = sqlite3_bind_int64(insert_session_stmt, 1, userId);
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

bool Database::updateSession(sessionIdType sessionId, std::string accessTokenHash, const char *refreshTokenHash)
{
        if (!valid)
                return false;
        auto fail = [&](int rc, int stepIndex)
        {
                std::cerr << "SQLite error (" << rc << ") on insertSession , step " << stepIndex << ": " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(update_session_stmt);
                return false;
        };
        int rc = sqlite3_bind_blob(update_session_stmt, 1, accessTokenHash.data(), accessTokenHash.size(), SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
                return fail(rc, 1);
        rc = sqlite3_bind_text(update_session_stmt, 2, refreshTokenHash, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
                return fail(rc, 2);
        rc = sqlite3_bind_int64(update_session_stmt, 3, sessionId);
        if (rc != SQLITE_OK)
                return fail(rc, 3);
        rc = sqlite3_step(update_session_stmt);
        if (rc != SQLITE_DONE)
                return fail(rc, 4);
        cleanup_stmt(update_session_stmt);
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

bool Database::getSessionFromId(sessionIdType sessionId, Session &session, bool &isFound)
{
        isFound = false;
        if (!valid)
                return false;
        int rc = sqlite3_bind_int64(get_session_from_id_stmt, 1, sessionId);
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
                session.sessionId = sqlite3_column_int64(get_session_from_id_stmt, 0);

                if (sqlite3_column_type(get_session_from_id_stmt, 1) != SQLITE_INTEGER)
                        goto type_error;
                session.userid = sqlite3_column_int64(get_session_from_id_stmt, 1);

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
                session.refreshTokenHash.assign(reinterpret_cast<const char *>(refreshTokenHashText), len);

                if (sqlite3_column_type(get_sessions_from_user_id_stmt, 4) != SQLITE_NULL)
                {
                        const unsigned char *fcm = sqlite3_column_text(get_sessions_from_user_id_stmt, 4);
                        session.fcmToken.assign(reinterpret_cast<const char *>(fcm));
                }
                else
                {
                        session.fcmToken = "";
                }
                if (sqlite3_column_type(get_session_from_id_stmt, 5) != SQLITE_INTEGER)
                        goto type_error;
                session.accessExpiry = sqlite3_column_int64(get_session_from_id_stmt, 5);

                if (sqlite3_column_type(get_session_from_id_stmt, 6) != SQLITE_INTEGER)
                        goto type_error;
                session.refreshExpiry = sqlite3_column_int64(get_session_from_id_stmt, 6);

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

bool Database::getSessionsFromUserId(userIdType userId, std::vector<Session> &sessions)
{
        if (!valid)
                return false;
        auto fail = [&](int rc, int stepIndex)
        {
                std::cerr << "SQLite error (" << rc << ") on getSessionsFromUserId , step " << stepIndex << ": " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(get_sessions_from_user_id_stmt);
                return false;
        };

        int rc = sqlite3_bind_int64(get_sessions_from_user_id_stmt, 1, userId);
        if (rc != SQLITE_OK)
                return fail(rc, 1);
        while (true)
        {
                rc = sqlite3_step(get_sessions_from_user_id_stmt);
                if (rc == SQLITE_DONE)
                {
                        cleanup_stmt(get_sessions_from_user_id_stmt);
                        return true;
                }
                else if (rc == SQLITE_ROW)
                {
                        Session session;
                        if (sqlite3_column_type(get_sessions_from_user_id_stmt, 0) != SQLITE_INTEGER)
                                goto type_error;
                        session.sessionId = sqlite3_column_int64(get_sessions_from_user_id_stmt, 0);

                        if (sqlite3_column_type(get_sessions_from_user_id_stmt, 1) != SQLITE_INTEGER)
                                goto type_error;
                        session.userid = sqlite3_column_int64(get_sessions_from_user_id_stmt, 1);

                        if (sqlite3_column_type(get_sessions_from_user_id_stmt, 2) != SQLITE_BLOB)
                                goto type_error;
                        int blobSize = sqlite3_column_bytes(get_sessions_from_user_id_stmt, 2);
                        sqlite3_column_blob(get_sessions_from_user_id_stmt, 2);
                        const char *blobPtr = reinterpret_cast<const char *>(sqlite3_column_blob(get_sessions_from_user_id_stmt, 2));
                        if (!blobPtr)
                                goto error;
                        session.accessTokenHash.assign(blobPtr, blobSize);

                        if (sqlite3_column_type(get_sessions_from_user_id_stmt, 3) != SQLITE_TEXT)
                                goto type_error;
                        int len = sqlite3_column_bytes(get_sessions_from_user_id_stmt, 3);
                        const unsigned char *refreshTokenHashText = sqlite3_column_text(get_sessions_from_user_id_stmt, 3);
                        if (!refreshTokenHashText)
                                goto error;
                        session.refreshTokenHash.assign(reinterpret_cast<const char *>(refreshTokenHashText), len);

                        if (sqlite3_column_type(get_sessions_from_user_id_stmt, 4) != SQLITE_NULL)
                        {
                                const unsigned char *fcm = sqlite3_column_text(get_sessions_from_user_id_stmt, 4);
                                session.fcmToken.assign(reinterpret_cast<const char *>(fcm));
                        }
                        else
                        {
                                session.fcmToken = "";
                        }

                        if (sqlite3_column_type(get_sessions_from_user_id_stmt, 5) != SQLITE_INTEGER)
                                goto type_error;
                        session.accessExpiry = sqlite3_column_int64(get_sessions_from_user_id_stmt, 5);

                        if (sqlite3_column_type(get_sessions_from_user_id_stmt, 6) != SQLITE_INTEGER)
                                goto type_error;
                        session.refreshExpiry = sqlite3_column_int64(get_sessions_from_user_id_stmt, 6);
                        sessions.push_back(session);
                }
                else
                {
                        cleanup_stmt(get_sessions_from_user_id_stmt);
                        return false;
                }
        }
error:
        std::cerr << "SQLite error (" << rc << ") on Database::getSessionsFromUserId : " << sqlite3_errmsg(db) << std::endl;
        cleanup_stmt(get_sessions_from_user_id_stmt);
        return false;
type_error:
        std::cerr << "SQLite error on Database::getSessionsFromUserId : incorrect type" << std::endl;
        cleanup_stmt(get_sessions_from_user_id_stmt);
        return false;
}

bool Database::updateFcmToken(sessionIdType sessionId, const char *fcmToken)
{
        if (!valid)
                return false;

        // Bind New Token
        if (fcmToken)
                sqlite3_bind_text(update_fcm_token_stmt, 1, fcmToken, -1, SQLITE_TRANSIENT);
        else
                sqlite3_bind_null(update_fcm_token_stmt, 1);

        // Bind Session ID
        sqlite3_bind_int64(update_fcm_token_stmt, 2, sessionId);

        int rc = sqlite3_step(update_fcm_token_stmt);
        if (rc != SQLITE_DONE)
        {
                std::cerr << "SQLite error on updateFcmToken: " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(update_fcm_token_stmt);
                return false;
        }

        cleanup_stmt(update_fcm_token_stmt);
        return true;
}

bool Database::removeFcmToken(const char *fcmToken)
{

        if (!valid)
                return false;

        // Bind New Token
        if (fcmToken)
                sqlite3_bind_text(remove_fcm_token_stmt, 1, fcmToken, -1, SQLITE_TRANSIENT);
        else
                return false;

        int rc = sqlite3_step(remove_fcm_token_stmt);
        if (rc != SQLITE_DONE)
        {
                std::cerr << "SQLite error on removeFcmToken: " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(remove_fcm_token_stmt);
                return false;
        }

        cleanup_stmt(remove_fcm_token_stmt);
        return true;
}
