#pragma once
#include "domain/TimeFormat.h"

#include <string>

namespace lms {

// A later deadline for one student on one assignment (FR-08, BR-03).
struct Extension {
    int assignmentId = 0;
    int studentId = 0;
    TimePoint newDeadline;
    std::string reason;
    int grantedBy = 0;  // the lecturer who granted it
    TimePoint grantedAt;
};

}  // namespace lms
