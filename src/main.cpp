#include "data/Database.h"
#include "data/DatabaseError.h"

#include <iostream>

int main() {
    try {
        lms::Database db("lms.db");
        db.initialiseSchema();

        // Step 3 check 1: all 10 tables exist.
        lms::Statement tables = db.prepare(
            "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table'");
        tables.step();
        std::cout << "Tables in lms.db: " << tables.columnInt(0) << "\n";

        // Step 3 check 2: a value with a quote is stored safely by bind(),
        // then the transaction is rolled back so nothing is kept.
        db.begin();
        lms::Statement insert = db.prepare("INSERT INTO department (name) VALUES (?)");
        insert.bind(1, std::string("O'Brien's Department'); DROP TABLE user; --"));
        insert.step();
        lms::Statement read = db.prepare("SELECT name FROM department WHERE id = ?");
        read.bind(1, db.lastInsertId());
        read.step();
        std::cout << "Stored safely: " << read.columnText(0) << "\n";
        db.rollback();

        lms::Statement count = db.prepare("SELECT COUNT(*) FROM department");
        count.step();
        std::cout << "Departments after rollback: " << count.columnInt(0) << "\n";

        // Step 3 check 3: a foreign key error becomes a DatabaseError, not a crash.
        lms::Statement bad = db.prepare(
            "INSERT INTO course (code, title, department_id) VALUES ('X1', 'Test', 999)");
        bad.step();
    } catch (const lms::DatabaseError& e) {
        std::cout << "Database error caught: " << e.what() << "\n";
    }
    return 0;
}
