#include "../database.h"

bool Database::prepareSessionRepository()
{
        const char *createSessionTableSQL =
            "CREATE TABLE IF NOT EXISTS sessions ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "userId INT NOT NULL,"
            "accessTokenHash TEXT,"
            "refreshTokenHash TEXT,"
            "accessExpires TIMESTAMP,"
            "refreshExpires TIMESTAMP"
            ");";
        int rc = sqlite3_exec(db, createSessionTableSQL, nullptr, nullptr, nullptr);
        if (!check(rc, "sql create session table error"))
                return false;
        return true;
}
