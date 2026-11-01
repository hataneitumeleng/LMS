#pragma once
#include "data/Statement.h"

#include <cstdint>
#include <string>

struct sqlite3;

namespace lms {

// Owns the connection to the SQLite database file (data access layer, Figure 1).
// Only the repositories use this class; the services and menus never see SQL.
class Database {
public:
    // Opens (or creates) the database file and switches on foreign keys.
    explicit Database(const std::string& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Creates all tables from sql/schema.sql if the database is new.
    void initialiseSchema();

    Statement prepare(const std::string& sql);

    // Transactions: everything between begin() and commit() is saved together,
    // or undone together by rollback() (NFR-07, NFR-08).
    void begin();
    void commit();
    void rollback();

    std::int64_t lastInsertId() const;

private:
    // For fixed SQL written in the program only, never for user input.
    void execute(const std::string& sql);
    // Brings a database made by an older version of the program up to date.
    void upgradeSchema();

    sqlite3* db_ = nullptr;
};

}  // namespace lms
