#pragma once
#include "data/Database.h"
#include "data/IAuditRepository.h"

namespace lms {

// Stores the audit_entry table.
class SqliteAuditRepository : public IAuditRepository {
public:
    explicit SqliteAuditRepository(Database& db);

    void add(const AuditEntry& entry) override;
    std::vector<AuditEntry> findByDepartment(int departmentId) override;
    std::vector<AuditEntry> findAll() override;

private:
    static AuditEntry readEntry(const Statement& row);

    Database& db_;
};

}  // namespace lms
