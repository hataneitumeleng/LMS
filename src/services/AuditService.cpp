#include "services/AuditService.h"

#include "domain/Errors.h"

namespace lms {

AuditService::AuditService(IAuditRepository& audit, const IClock& clock)
    : audit_(audit), clock_(clock) {}

void AuditService::record(int userId, int submissionId, std::optional<double> oldValue,
                          double newValue, const std::string& reason) {
    audit_.add(AuditEntry{0, submissionId, userId, clock_.now(), oldValue, newValue, reason});
}

std::vector<AuditEntry> AuditService::entriesFor(const User& viewer) {
    if (viewer.role() == Role::Administrator) return audit_.findAll();
    if (viewer.role() == Role::HoD) return audit_.findByDepartment(*viewer.departmentId());
    throw AuthError("Only an HoD or an administrator can view the audit trail");
}

}  // namespace lms
