#include "database.h"
#include <iostream>
#include <vector>

const char *mainDatabasePath = "../Databases/zchat_data.sqlite";

Database::Database()
{

        int rc = sqlite3_open(mainDatabasePath, &db);
        if (!check(rc, "sql db open error"))
                return;

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
        if (!check(rc, "sql create account table error"))
                return;

        if (!prepare(insert_email_account_stmt,
                     "INSERT INTO accounts(username, email, passwordHash, googleId) VALUES (?1 , ?2 , ?3 , ?4 );",
                     "sql insert email statement prepare error"))
                return;
        if (!prepare(check_email_exists_stmt,
                     "SELECT email FROM accounts WHERE email = ?1 LIMIT 1;",
                     "sql check email statement prepare error"))
                return;
        if (!prepare(check_username_exists_stmt,
                     "SELECT username FROM accounts WHERE username = ?1 LIMIT 1;",
                     "sql check username statement prepare error"))
                return;
        if (!prepare(get_pass_hash_from_email_stmt,
                     "SELECT passwordHash FROM accounts WHERE email = ?1 LIMIT 1;",
                     "sql get passwordHash statement prepare error"))
                return;
        if (!prepare(check_google_id_exists_stmt,
                     "SELECT googleId FROM accounts WHERE googleId = ?1 LIMIT 1;",
                     "sql check googleId statement prepare error"))
                return;
        valid = true;
}

bool Database::prepare(sqlite3_stmt *&stmt, const char *sql, const char *name)
{
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        return check(rc, name);
}

bool Database::check(int rc, const char *context)
{
        if (rc != SQLITE_OK)
        {
                std::cerr << context << " (" << rc << ") : "
                          << sqlite3_errmsg(db) << std::endl;
                if (db)
                        sqlite3_close(db);
                db = nullptr;
                return false;
        }
        return true;
}

Database::~Database()
{
        if (insert_email_account_stmt)
                sqlite3_finalize(insert_email_account_stmt);
        if (db)
                sqlite3_close(db);
}

void cleanup_stmt(sqlite3_stmt *stmt)
{
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
}

std::string Database::getPasswordHash(const char *email, bool &found, bool &status)
{
        status = false;
        found = false;
        if (!valid)
                return {};

        int rc = sqlite3_bind_text(get_pass_hash_from_email_stmt, 1, email, -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
                goto error;
        rc = sqlite3_step(get_pass_hash_from_email_stmt);
        if (rc == SQLITE_ROW)
        {
                if (sqlite3_column_type(get_pass_hash_from_email_stmt, 0) != SQLITE_TEXT)
                        goto error;
                const unsigned char *text = sqlite3_column_text(get_pass_hash_from_email_stmt, 0);
                int len = sqlite3_column_bytes(get_pass_hash_from_email_stmt, 0);

                std::string hash(reinterpret_cast<const char *>(text), len);
                found = true;
                status = true;
                cleanup_stmt(get_pass_hash_from_email_stmt);
                return hash;
        }
        else if (rc == SQLITE_DONE)
        {
                found = false;
                status = true;
                cleanup_stmt(get_pass_hash_from_email_stmt);
                return {};
        }
error:
        std::cerr << "SQLite error (" << rc << ") on Database::getPasswordHash : " << sqlite3_errmsg(db) << std::endl;
        cleanup_stmt(get_pass_hash_from_email_stmt);
        return {};
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
                cleanup_stmt(insert_email_account_stmt);
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

        cleanup_stmt(insert_email_account_stmt);
        return true;
}