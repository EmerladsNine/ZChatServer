#include "../database.h"

bool Database::prepareMessagesRepository()
{
        const char *createMessagesTableSQL =
            "CREATE TABLE IF NOT EXISTS messages ("
            "senderUserId INT NOT NULL,"
            "receiverSessionId INT NOT NULL,"
            "message TEXT NOT NULL,"
            "timestamp INT NOT NULL"
            ");";
        int rc = sqlite3_exec(db, createMessagesTableSQL, nullptr, nullptr, nullptr);
        if (!check(rc, "sql create messages table error"))
                return false;
        return true;
}