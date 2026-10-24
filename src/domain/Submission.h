#pragma once
#include "domain/SubmissionStatus.h"
#include "domain/TimeFormat.h"

#include <optional>
#include <string>

namespace lms {

// One student's work for one assignment (FR-10). The files are in SubmissionVersion;
// this holds the status, days late and, from step 5d on, the marks.
struct Submission {
    int id = 0;  // 0 until saved
    int assignmentId = 0;
    int studentId = 0;
    SubmissionStatus status = SubmissionStatus::Submitted;
    int daysLate = 0;
    std::optional<double> rawMark;
    std::optional<double> finalMark;
    std::string feedback;
    std::optional<int> markedBy;
    std::optional<int> moderatedBy;
    std::optional<TimePoint> moderatedAt;

    // True until it is marked (Submitted or Late).
    bool isOpen() const;
    // BR-05, BR-06: only before the deadline, and only if not marked yet.
    bool canResubmit(TimePoint now, TimePoint deadline) const;
    // Sets Submitted (0 days) or Late (1 or more days). Only allowed while open.
    void setLateness(int days);
};

}  // namespace lms
