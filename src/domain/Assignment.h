#pragma once
#include "domain/Extension.h"
#include "domain/TimeFormat.h"

#include <optional>
#include <string>

namespace lms {

// An assignment set by a lecturer for one course (FR-07).
struct Assignment {
    int id = 0;  // 0 until saved
    int courseId = 0;
    std::string title;
    std::string description;
    TimePoint deadline;
    int maxMark = 0;  // 1 to 100
    int createdBy = 0;
    std::string briefPath;  // the lecturer's brief file in the program's folder; empty if none

    // The deadline that applies to one student: their extension if they have one,
    // otherwise the normal deadline (BR-03).
    TimePoint effectiveDeadline(const std::optional<Extension>& extension) const {
        return extension ? extension->newDeadline : deadline;
    }
};

}  // namespace lms
