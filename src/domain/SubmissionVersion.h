#pragma once
#include "domain/TimeFormat.h"

#include <string>

namespace lms {

// One uploaded file. Every resubmission adds a version; old ones are kept (BR-05).
struct SubmissionVersion {
    int versionNo = 1;
    std::string filePath;  // the copy inside the program's submissions folder
    TimePoint submittedAt;
};

}  // namespace lms
