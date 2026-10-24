#pragma once
#include "data/Database.h"

namespace lms {

// RAII guard for a database transaction. The constructor calls begin().
// If commit() is not reached (for example because an exception was thrown),
// the destructor calls rollback(), so nothing is half-saved (NFR-07, NFR-08).
class Transaction {
public:
    explicit Transaction(Database& db) : db_(db) { db_.begin(); }
    ~Transaction() {
        if (!committed_) db_.rollback();
    }

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    void commit() {
        db_.commit();
        committed_ = true;
    }

private:
    Database& db_;
    bool committed_ = false;
};

}  // namespace lms
