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
        if (!prepare(get_last_inserted_id_stmt,
                     "SELECT last_insert_rowid();",
                     "sql get lastInsertedId statement prepare error"))\
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

bool Database::getLastInsertedId(int &id)
{
        if (!valid)
                return false;
        int rc = sqlite3_step(get_last_inserted_id_stmt);
        if (rc == SQLITE_ROW)
        {
                if (sqlite3_column_type(get_last_inserted_id_stmt, 0) != SQLITE_INTEGER)
                        goto error;
                id = sqlite3_column_int(get_last_inserted_id_stmt, 0);
                cleanup_stmt(get_last_inserted_id_stmt);
                return true;
        }
error:
        std::cerr << "SQLite error (" << rc << ") on Database::getLastInsertedId : " << sqlite3_errmsg(db) << std::endl;
        cleanup_stmt(get_last_inserted_id_stmt);
        return false;
}