#include "data/Database.h"

#include "data/DatabaseError.h"
#include "data/Schema.h"

#include <sqlite3.h>

namespace lms {

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::string message = db_ ? sqlite3_errmsg(db_) : "out of memory";
        sqlite3_close(db_);
        throw DatabaseError("Cannot open database '" + path + "': " + message);
    }
    sqlite3_busy_timeout(db_, 2000);
    execute("PRAGMA foreign_keys = ON;");  // SQLite has foreign keys off by default
}

Database::~Database() {
    sqlite3_close(db_);
}

void Database::initialiseSchema() {
    Statement check = prepare(
        "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = 'user'");
    check.step();
    if (check.columnInt(0) == 0) {
        begin();
        try {
            execute(kSchemaSql);
            commit();
        } catch (...) {
            rollback();
            throw;
        }
    }
}

Statement Database::prepare(const std::string& sql) {
    return Statement(db_, sql);
}

void Database::begin() {
    execute("BEGIN;");
}

void Database::commit() {
    execute("COMMIT;");
}

void Database::rollback() {
    // Never throws: rollback is called while another error is being handled.
    sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
}

std::int64_t Database::lastInsertId() const {
    return sqlite3_last_insert_rowid(db_);
}

void Database::execute(const std::string& sql) {
    char* error = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        std::string message = error ? error : "unknown error";
        sqlite3_free(error);
        throw DatabaseError("SQL failed: " + message);
    }
}

}  // namespace lms
