#include "../database.h"
#include <iostream>

bool Database::prepareAccountRepository()
{
        const char *createAccountTableSQL =
            "CREATE TABLE IF NOT EXISTS accounts ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT,"
            "email TEXT,"
            "passwordHash TEXT,"
            "googleId TEXT,"
            "sessionListVersion INTEGER DEFAULT 0"
            ");";
        int rc = sqlite3_exec(db, createAccountTableSQL, nullptr, nullptr, nullptr);
        if (!check(rc, "sql create account table error"))
                return false;

        if (!prepare(insert_account_stmt,
                     "INSERT INTO accounts(username, email, passwordHash, googleId) VALUES (?1 , ?2 , ?3 , ?4);",
                     "sql insert account statement prepare error"))
                return false;
        if (!prepare(check_email_exists_stmt,
                     "SELECT email FROM accounts WHERE email = ?1 LIMIT 1;",
                     "sql check email statement prepare error"))
                return false;
        if (!prepare(get_account_from_username_stmt,
                     "SELECT * FROM accounts WHERE username = ?1 LIMIT 1;",
                     "sql get account from username statement prepare error"))
                return false;
        if (!prepare(get_account_from_id_stmt,
                     "SELECT * FROM accounts WHERE id = ?1 LIMIT 1;",
                     "sql get account from id statement prepare error"))
                return false;
        if (!prepare(get_account_from_email_stmt,
                     "SELECT * FROM accounts WHERE email = ?1 LIMIT 1;",
                     "sql get account from email statement prepare error"))
                return false;
        if (!prepare(get_account_from_google_id_stmt,
                     "SELECT * FROM accounts WHERE googleId = ?1 LIMIT 1;",
                     "sql get account from googelId statement prepare error"))
                return false;
        if (!prepare(check_google_id_exists_stmt,
                     "SELECT googleId FROM accounts WHERE googleId = ?1 LIMIT 1;",
                     "sql check googleId statement prepare error"))
                return false;
        return true;
}

void Database::objExists(sqlite3_stmt *exists_stmt, const char *obj, bool &out, bool &status)
{
        status = false;
        if (!valid)
                return;
        int rc = sqlite3_bind_text(exists_stmt, 1, obj, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
        {
                std::cerr << "SQLite error (" << rc << ") on Database::objExists binding obj param : " << sqlite3_errmsg(db) << std::endl;
                return;
        }

        rc = sqlite3_step(exists_stmt);
        if (rc == SQLITE_ROW)
        {
                out = true;
        }
        else if (rc == SQLITE_DONE)
        {
                out = false;
        }
        else
        {
                std::cerr << "SQLite error (" << rc << ") on Database::objExists sqlite3_step : " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(exists_stmt);
                return;
        }
        cleanup_stmt(exists_stmt);
        status = true;
        return;
}

bool Database::insertEmailAccount(const char *username, const char *email, const char *passwordHash)
{
        if (!valid)
                return false;

        auto fail = [&](int rc, int stepIndex)
        {
                std::cerr << "SQLite error (" << rc << ") on insertEmailAccount , step " << stepIndex << ": " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(insert_account_stmt);
                return false;
        };

        // Binding Parameters.

        struct
        {
                int index;
                const char *value;
        } params[] = {
            {1, username},
            {2, email},
            {3, passwordHash}};

        for (auto &param : params)
        {
                int rc = sqlite3_bind_text(insert_account_stmt, param.index, param.value, -1, SQLITE_TRANSIENT);
                if (rc != SQLITE_OK)
                        return fail(rc, param.index);
        }

        const int googleIdParamIndex = 4;
        int rc = sqlite3_bind_null(insert_account_stmt, googleIdParamIndex);
        if (rc != SQLITE_OK)
                return fail(rc, 4);

        // Running Statement.
        rc = sqlite3_step(insert_account_stmt);
        if (rc != SQLITE_DONE)
                return fail(rc, 5);

        cleanup_stmt(insert_account_stmt);
        return true;
}

bool Database::insertGoogleAccount(const char *username, const char *googleId)
{
        if (!valid)
                return false;
        auto fail = [&](int rc, int stepIndex)
        {
                std::cerr << "SQLite error (" << rc << ") on insertEmailAccount , step " << stepIndex << ": " << sqlite3_errmsg(db) << std::endl;
                cleanup_stmt(insert_account_stmt);
                return false;
        };

        // Binding Parameters.
        int rc = sqlite3_bind_text(insert_account_stmt, 1, username, -1, SQLITE_TRANSIENT); // Username
        if (rc != SQLITE_OK)
                return fail(rc, 1);

        rc = sqlite3_bind_null(insert_account_stmt, 2); // Email
        if (rc != SQLITE_OK)
                return fail(rc, 2);

        rc = sqlite3_bind_null(insert_account_stmt, 3); // Password
        if (rc != SQLITE_OK)
                return fail(rc, 3);

        rc = sqlite3_bind_text(insert_account_stmt, 4, googleId, -1, SQLITE_TRANSIENT); // GoogleId
        if (rc != SQLITE_OK)
                return fail(rc, 4);

        // Running Statement.
        rc = sqlite3_step(insert_account_stmt);
        if (rc != SQLITE_DONE)
                return fail(rc, 5);

        cleanup_stmt(insert_account_stmt);
        return true;
}

bool Database::getAccount(sqlite3_stmt *readyToRunStmt, Account &accountOut, bool &isFound)
{
        int rc = sqlite3_step(readyToRunStmt);
        if (rc == SQLITE_DONE)
        {
                isFound = false;
                cleanup_stmt(readyToRunStmt);
                return true;
        }
        else if (rc == SQLITE_ROW)
        {
                if (sqlite3_column_type(readyToRunStmt, 0) != SQLITE_INTEGER)
                        goto type_error;
                userIdType id = sqlite3_column_int64(readyToRunStmt, 0);

                if (sqlite3_column_type(readyToRunStmt, 1) != SQLITE_TEXT)
                        goto type_error;
                const unsigned char *text = sqlite3_column_text(readyToRunStmt, 1);
                int len = sqlite3_column_bytes(readyToRunStmt, 1);
                std::string username(reinterpret_cast<const char *>(text), len);

                std::string email;
                if (sqlite3_column_type(readyToRunStmt, 2) == SQLITE_NULL)
                {
                        email.clear();
                }
                else if (sqlite3_column_type(readyToRunStmt, 2) == SQLITE_TEXT)
                {
                        const unsigned char *text = sqlite3_column_text(readyToRunStmt, 2);
                        if (!text)
                                goto error;

                        int len = sqlite3_column_bytes(readyToRunStmt, 2);
                        email.assign(reinterpret_cast<const char *>(text), len);
                }
                else
                {
                        goto type_error;
                }

                std::string passHash;
                if (sqlite3_column_type(readyToRunStmt, 3) == SQLITE_NULL)
                {
                        passHash.clear();
                }
                else if (sqlite3_column_type(readyToRunStmt, 3) == SQLITE_TEXT)
                {
                        const unsigned char *text = sqlite3_column_text(readyToRunStmt, 3);
                        if (!text)
                                goto error;

                        int len = sqlite3_column_bytes(readyToRunStmt, 3);
                        passHash.assign(reinterpret_cast<const char *>(text), len);
                }
                else
                {
                        goto type_error;
                }

                std::string googleId;
                if (sqlite3_column_type(readyToRunStmt, 4) == SQLITE_NULL)
                {
                        googleId.clear();
                }
                else if (sqlite3_column_type(readyToRunStmt, 4) == SQLITE_TEXT)
                {
                        const unsigned char *text = sqlite3_column_text(readyToRunStmt, 4);
                        if (!text)
                                goto error;

                        int len = sqlite3_column_bytes(readyToRunStmt, 4);
                        googleId.assign(reinterpret_cast<const char *>(text), len);
                }
                else
                {
                        goto type_error;
                }

                if (sqlite3_column_type(readyToRunStmt, 5) != SQLITE_INTEGER)
                        goto type_error;
                sessionListVersionType sessionListVersion = sqlite3_column_int64(readyToRunStmt, 5);

                accountOut.id = id;
                accountOut.username = username;
                accountOut.email = email;
                accountOut.passHash = passHash;
                accountOut.googleId = googleId;
                accountOut.sessionListVersion = sessionListVersion;
                isFound = true;
                cleanup_stmt(readyToRunStmt);
                return true;
        }
error:
        std::cerr << "SQLite error (" << rc << ") on Database::getAccount : " << sqlite3_errmsg(db) << std::endl;
        cleanup_stmt(readyToRunStmt);
        return false;
type_error:
        std::cerr << "SQLite error on Database::getAccount : incorrect type" << std::endl;
        cleanup_stmt(readyToRunStmt);
        return false;
}

bool Database::getAccountFromUsername(const char *username, Account &accountOut, bool &isFound)
{
        isFound = false;
        if (!valid)
                return false;

        int rc = sqlite3_bind_text(get_account_from_username_stmt, 1, username, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
                goto error;
        return getAccount(get_account_from_username_stmt, accountOut, isFound);
error:
        std::cerr << "SQLite error (" << rc << ") on Database::getAccountFromUsername : " << sqlite3_errmsg(db) << std::endl;
        cleanup_stmt(get_account_from_username_stmt);
        return false;
}

bool Database::getAccountFromId(userIdType id, Account &accountOut, bool &isFound)
{
        isFound = false;
        if (!valid)
                return false;

        int rc = sqlite3_bind_int64(get_account_from_id_stmt, 1, id);
        if (rc != SQLITE_OK)
                goto error;
        return getAccount(get_account_from_id_stmt, accountOut, isFound);
error:
        std::cerr << "SQLite error (" << rc << ") on Database::getAccountFromId : " << sqlite3_errmsg(db) << std::endl;
        cleanup_stmt(get_account_from_id_stmt);
        return false;
}

bool Database::getAccountFromEmail(const char *email, Account &accountOut, bool &isFound)
{
        isFound = false;
        if (!valid)
                return false;

        int rc = sqlite3_bind_text(get_account_from_email_stmt, 1, email, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
                goto error;
        return getAccount(get_account_from_email_stmt, accountOut, isFound);
error:
        std::cerr << "SQLite error (" << rc << ") on Database::getAccountFromEmail : " << sqlite3_errmsg(db) << std::endl;
        cleanup_stmt(get_account_from_email_stmt);
        return false;
}

bool Database::getAccountFromGoogleId(const char *googleId, Account &accountOut, bool &isFound)
{
        isFound = false;
        if (!valid)
                return false;

        int rc = sqlite3_bind_text(get_account_from_google_id_stmt, 1, googleId, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
                goto error;
        return getAccount(get_account_from_google_id_stmt, accountOut, isFound);
error:
        std::cerr << "SQLite error (" << rc << ") on Database::getAccountFromEmail : " << sqlite3_errmsg(db) << std::endl;
        cleanup_stmt(get_account_from_google_id_stmt);
        return false;
}