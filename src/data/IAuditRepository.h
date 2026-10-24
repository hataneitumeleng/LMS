#pragma once
#include "domain/AuditEntry.h"

#include <vector>

namespace lms {

// The audit trail of mark changes (FR-17). Entries are only ever added, never changed.
class IAuditRepository {
public:
    virtual ~IAuditRepository() = default;
    virtual void add(const AuditEntry& entry) = 0;
    // Entries for submissions to assignments of courses in one department.
    virtual std::vector<AuditEntry> findByDepartment(int departmentId) = 0;
    virtual std::vector<AuditEntry> findAll() = 0;
};

}  // namespace lms
