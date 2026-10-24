#pragma once
#include "domain/TimeFormat.h"

#include <optional>
#include <string>

namespace lms {

// One mark change: who, when, old value, new value and why (FR-17, NFR-08).
struct AuditEntry {
    int id = 0;  // 0 until saved
    int submissionId = 0;
    int userId = 0;
    TimePoint changedAt;
    std::optional<double> oldValue;  // empty for the first mark
    double newValue = 0.0;
    std::string reason;
};

}  // namespace lms
