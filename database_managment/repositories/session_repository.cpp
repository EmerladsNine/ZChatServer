#include "../database.h"
#include <iostream>

bool Database::prepareSessionRepository()
{
        const char *createSessionTableSQL =
            "CREATE TABLE IF NOT EXISTS sessions ("
            "userId INT NOT NULL,"
            "accessTokenHash BLOB NOT NULL,"
            "refreshTokenHash TEXT NOT NULL,"
            "accessExpires TIMESTAMP,"
            "refreshExpires TIMESTAMP"
            ");";
        int rc = sqlite3_exec(db, createSessionTableSQL, nullptr, nullptr, nullptr);
        if (!check(rc, "sql create session table error"))
                return false;
        if (!prepare(insert_session_stmt,
                     "INSERT INTO sessions(userId, accessTokenHash, refreshTokenHash, accessExpires, refreshExpires) VALUES (?1 , ?2 , ?3 , "
                     "datetime('now','15 minutes'),"
                     "datetime('now','30 days')"
                     ");",
                     "sql insert session statement prepare error"))
                return false;
        return true;
}

bool Database::insertSession(int userId,std::string accessTokenHash,const char* refreshTokenHash)
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
