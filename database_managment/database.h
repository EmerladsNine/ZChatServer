#pragma once
#include <string>
#include <sqlite3.h>
#include "data/account.h"
#include "data/session.h"

class Database
{
private:
public:
        sqlite3 *db = nullptr;
        bool valid = false;

        // Account Repository
        sqlite3_stmt *insert_account_stmt = nullptr;
        sqlite3_stmt *check_email_exists_stmt = nullptr;
        sqlite3_stmt *get_account_from_username_stmt = nullptr;
        sqlite3_stmt *get_account_from_id_stmt = nullptr;
        sqlite3_stmt *get_account_from_email_stmt = nullptr;
        sqlite3_stmt *get_account_from_google_id_stmt = nullptr;
        sqlite3_stmt *check_google_id_exists_stmt = nullptr;
        sqlite3_stmt *get_last_inserted_id_stmt = nullptr;
        // Session Repository
        sqlite3_stmt *insert_session_stmt = nullptr;
        sqlite3_stmt *check_session_exist_stmt = nullptr;
        sqlite3_stmt *get_session_from_id_stmt = nullptr;

        Database();
        bool prepare(sqlite3_stmt *&stmt, const char *sql, const char *name);
        bool check(int rc, const char *context);
        void cleanup_stmt(sqlite3_stmt *stmt);
        bool getLastInsertedId(int &id);
        ~Database();

        // Account Repository
        bool prepareAccountRepository();
        void objExists(sqlite3_stmt *exists_stmt, const char *obj, bool &out, bool &status);
        bool insertEmailAccount(const char *username, const char *email, const char *passwordHash);
        bool insertGoogleAccount(const char *username, const char *googleId);
        bool getAccount(sqlite3_stmt *readyToRunStmt, Account &accountOut, bool &isFound);
        bool getAccountFromEmail(const char *email, Account &accountOut, bool &isFound);
        bool getAccountFromGoogleId(const char *googleId, Account &accountOut, bool &isFound);
        bool getAccountFromUsername(const char *username, Account &accountOut, bool &isFound);
        bool getAccountFromId(int id, Account &accountOut, bool &isFound);

        // Token Respository
        bool prepareSessionRepository();
        bool insertSession(int userId, std::string accessTokenHash, const char *refreshTokenHash);
        bool sessionExists(std::string accessTokenHash, const char *refreshTokenHash, bool &result);
        bool getSessionFromId(int sessionId,Session& session,bool& isFound);
};
