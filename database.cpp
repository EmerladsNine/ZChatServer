#include "database.h"
#include <iostream>
#include <vector>

const char *mainDatabasePath = "../Databases/zchat_data.sqlite";

Database::Database()
{

        int rc = sqlite3_open(mainDatabasePath, &db);
        if (rc != SQLITE_OK)
        {
                std::cerr << "sql db open error(" << rc << ") : " << sqlite3_errmsg(db) << std::endl;
                if (db)
                        sqlite3_close(db);
                db = nullptr;
                return;
        }

        // Create account table if it doesnt exist.
        const char *createAccountTableSQL =
            "CREATE TABLE IF NOT EXISTS accounts ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT,"
            "email TEXT,"
            "passwordHash TEXT,"
            "googleId INTEGER"
            ");";
        rc = sqlite3_exec(db, createAccountTableSQL, nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK)
        {
                std::cerr << "sql create account table error(" << rc << ") : " << sqlite3_errmsg(db) << std::endl;
        }

        const char *insert_email_account_stmt_text =
            "INSERT INTO accounts(username, email, passwordHash, googleId) "
            "VALUES (?1 , ?2 , ?3 , ?4 );";
        rc = sqlite3_prepare_v2(db, insert_email_account_stmt_text, -1, &insert_email_account_stmt, nullptr);
        if (rc != SQLITE_OK)
        {
                std::cerr << "sql insert email statement prepare error(" << rc << ") : " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                db = nullptr;
                return;
        }

        const char *check_email_exists_stmt_text =
            "SELECT email FROM accounts WHERE email = ?1 LIMIT 1;";
        rc = sqlite3_prepare_v2(db, check_email_exists_stmt_text, -1, &check_email_exists_stmt, nullptr);
        if (rc != SQLITE_OK)
        {
                std::cerr << "sql check email statement prepare error(" << rc << ") : " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                db = nullptr;
                return;
        }

        const char *check_username_exists_stmt_text =
            "SELECT username FROM accounts WHERE username = ?1 LIMIT 1;";
        rc = sqlite3_prepare_v2(db, check_username_exists_stmt_text, -1, &check_username_exists_stmt, nullptr);
        if (rc != SQLITE_OK)
        {
                std::cerr << "sql check username statement prepare error(" << rc << ") : " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                db = nullptr;
                return;
        }

        valid = true;
}

Database::~Database()
{
        if (insert_email_account_stmt)
                sqlite3_finalize(insert_email_account_stmt);
        if (db)
                sqlite3_close(db);
}

void cleanup(sqlite3_stmt *stmt)
{
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
}

bool Database::objExists(sqlite3_stmt *exists_stmt, const char *obj, bool &out)
{
        if (!valid)
                return false;
        int rc = sqlite3_bind_text(exists_stmt, 1, obj, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
        {
                std::cerr << "SQLite error (" << rc << ") on Database::objExists binding obj param : " << sqlite3_errmsg(db) << std::endl;
                return false;
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
                cleanup(exists_stmt);
                return false;
        }
        cleanup(exists_stmt);
        return true;
}

bool Database::insertEmailAccount(const char *username, const char *email, const char *passwordHash)
{
        if (!valid)
                return false;

        auto fail = [&](int rc, int stepIndex)
        {
                std::cerr << "SQLite error (" << rc << ") on insertEmailAccount , step " << stepIndex << ": " << sqlite3_errmsg(db) << std::endl;
                cleanup(insert_email_account_stmt);
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
                int rc = sqlite3_bind_text(insert_email_account_stmt, param.index, param.value, -1, SQLITE_TRANSIENT);
                if (rc != SQLITE_OK)
                        return fail(rc, param.index);
        }

        const int googleIdParamIndex = 4;
        int rc = sqlite3_bind_null(insert_email_account_stmt, googleIdParamIndex);
        if (rc != SQLITE_OK)
                return fail(rc, 4);

        // Running Statement.

        rc = sqlite3_step(insert_email_account_stmt);
        if (rc != SQLITE_DONE)
                return fail(rc, 5);

        cleanup(insert_email_account_stmt);
        return true;
}