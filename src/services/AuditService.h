#pragma once
#include "data/IAuditRepository.h"
#include "domain/User.h"
#include "services/IClock.h"

#include <optional>
#include <string>
#include <vector>

namespace lms {

// Records every mark change and shows the trail to the people allowed to see it (FR-17).
class AuditService {
public:
    AuditService(IAuditRepository& audit, const IClock& clock);

    // Call this inside the same transaction as the mark change (NFR-08).
    void record(int userId, int submissionId, std::optional<double> oldValue, double newValue,
                const std::string& reason);

    // An administrator sees everything, an HoD sees their own department, anyone else is refused.
    std::vector<AuditEntry> entriesFor(const User& viewer);

private:
    IAuditRepository& audit_;
    const IClock& clock_;
};

}  // namespace lms
