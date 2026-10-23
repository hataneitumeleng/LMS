#include "data/Statement.h"

#include "data/DatabaseError.h"

#include <sqlite3.h>
#include <utility>

namespace lms {

Statement::Statement(sqlite3* db, const std::string& sql) : db_(db) {
    check(sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt_, nullptr), "prepare");
}

Statement::~Statement() {
    sqlite3_finalize(stmt_);  // safe to call with nullptr
}

Statement::Statement(Statement&& other) noexcept
    : db_(other.db_), stmt_(std::exchange(other.stmt_, nullptr)) {}

Statement& Statement::operator=(Statement&& other) noexcept {
    if (this != &other) {
        sqlite3_finalize(stmt_);
        db_ = other.db_;
        stmt_ = std::exchange(other.stmt_, nullptr);
    }
    return *this;
}

Statement& Statement::bind(int index, int value) {
    check(sqlite3_bind_int(stmt_, index, value), "bind");
    return *this;
}

Statement& Statement::bind(int index, std::int64_t value) {
    check(sqlite3_bind_int64(stmt_, index, value), "bind");
    return *this;
}

Statement& Statement::bind(int index, double value) {
    check(sqlite3_bind_double(stmt_, index, value), "bind");
    return *this;
}

Statement& Statement::bind(int index, const std::string& value) {
    // SQLITE_TRANSIENT makes SQLite keep its own copy of the text.
    check(sqlite3_bind_text(stmt_, index, value.c_str(), static_cast<int>(value.size()),
                            SQLITE_TRANSIENT),
          "bind");
    return *this;
}

Statement& Statement::bindNull(int index) {
    check(sqlite3_bind_null(stmt_, index), "bind");
    return *this;
}

bool Statement::step() {
    int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    check(rc, "step");  // throws
    return false;
}

void Statement::reset() {
    sqlite3_reset(stmt_);
    sqlite3_clear_bindings(stmt_);
}

int Statement::columnInt(int column) const {
    return sqlite3_column_int(stmt_, column);
}

std::int64_t Statement::columnInt64(int column) const {
    return sqlite3_column_int64(stmt_, column);
}

double Statement::columnDouble(int column) const {
    return sqlite3_column_double(stmt_, column);
}

std::string Statement::columnText(int column) const {
    const unsigned char* text = sqlite3_column_text(stmt_, column);
    return text ? reinterpret_cast<const char*>(text) : "";
}

bool Statement::columnIsNull(int column) const {
    return sqlite3_column_type(stmt_, column) == SQLITE_NULL;
}

void Statement::check(int resultCode, const char* action) const {
    if (resultCode != SQLITE_OK) {
        throw DatabaseError(std::string("SQL ") + action + " failed: " + sqlite3_errmsg(db_));
    }
}

}  // namespace lms
