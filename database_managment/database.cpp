#include "database.h"
#include <iostream>
#include <vector>

const char *mainDatabasePath = "../Databases/zchat_data.sqlite";

Database::Database()
{

        int rc = sqlite3_open(mainDatabasePath, &db);
        if (!check(rc, "sql db open error"))
                return;
        if (!prepareAccountRepository())
                return;
        if (!prepareSessionRepository())
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

void Database::cleanup_stmt(sqlite3_stmt *stmt)
{
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
}

Database::~Database()
{
        sqlite3_finalize(insert_account_stmt);
        sqlite3_finalize(check_email_exists_stmt);
        sqlite3_finalize(get_account_from_username_stmt);
        sqlite3_finalize(get_account_from_id_stmt);
        sqlite3_finalize(get_account_from_email_stmt);
        sqlite3_finalize(check_google_id_exists_stmt);
        sqlite3_close(db);
}