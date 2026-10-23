#pragma once
#include <cstdint>
#include <string>

struct sqlite3;
struct sqlite3_stmt;

namespace lms {

// One prepared SQL statement. Values are attached with bind(), never pasted into
// the SQL text, so user input cannot change the query (SQL injection).
// The statement is finalised automatically when the object goes out of scope (RAII).
class Statement {
public:
    Statement(sqlite3* db, const std::string& sql);
    ~Statement();

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&& other) noexcept;

    // Parameters are numbered from 1, matching the ? marks in the SQL.
    Statement& bind(int index, int value);
    Statement& bind(int index, std::int64_t value);
    Statement& bind(int index, double value);
    Statement& bind(int index, const std::string& value);
    Statement& bindNull(int index);

    // Runs the statement. Returns true while there is a result row to read.
    bool step();
    // Lets the same statement run again with new values.
    void reset();

    // Columns are numbered from 0, in the order of the SELECT list.
    int columnInt(int column) const;
    std::int64_t columnInt64(int column) const;
    double columnDouble(int column) const;
    std::string columnText(int column) const;
    bool columnIsNull(int column) const;

private:
    void check(int resultCode, const char* action) const;

    sqlite3* db_ = nullptr;
    sqlite3_stmt* stmt_ = nullptr;
};

}  // namespace lms
