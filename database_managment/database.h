#pragma once
#include <string>
#include <sqlite3.h>
#include "account.h"

class Database
{
private:
public:
        sqlite3 *db = nullptr;
        sqlite3_stmt *insert_account_stmt = nullptr;
        sqlite3_stmt *check_email_exists_stmt = nullptr;
        sqlite3_stmt *get_account_from_username_stmt = nullptr;
        sqlite3_stmt *get_account_from_id_stmt = nullptr;
        sqlite3_stmt *check_google_id_exists_stmt = nullptr;
        sqlite3_stmt *get_pass_hash_from_email_stmt = nullptr;

        bool valid = false;

        Database();
        bool prepare(sqlite3_stmt *&stmt, const char *sql, const char *name);
        std::string getPasswordHash(const char *email, bool &found, bool &status);
        bool check(int rc, const char *context);
        void objExists(sqlite3_stmt *exists_stmt, const char *obj, bool &out, bool &status);
        bool insertEmailAccount(const char *username, const char *email, const char *passwordHash);
        bool insertGoogleAccount(const char *username, const char *googleId);
        bool getAccount(sqlite3_stmt *readyToRunStmt, Account &accountOut, bool &isFound);
        bool getAccountFromUsername(const char *username, Account &accountOut, bool &isFound);
        bool getAccountFromId(int id, Account &accountOut, bool &isFound);

        ~Database();
};
