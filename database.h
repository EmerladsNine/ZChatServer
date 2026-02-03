#pragma once
#include <string>
#include <sqlite3.h>

class Database
{
private:
public:
        sqlite3 *db = nullptr;
        sqlite3_stmt *insert_email_account_stmt = nullptr;
        sqlite3_stmt *check_email_exists_stmt = nullptr;
        sqlite3_stmt *check_username_exists_stmt = nullptr;

        bool valid = false;

        Database();
        bool objExists(sqlite3_stmt *exists_stmt, const char *obj, bool &out);
        bool insertEmailAccount(const char *username, const char *email, const char *passwordHash);
        ~Database();
};
