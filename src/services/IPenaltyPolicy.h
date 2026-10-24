#pragma once
#include "domain/TimeFormat.h"

namespace lms {

// Strategy pattern: the late-penalty rule (BR-01, BR-02) sits behind this interface,
// so the HoD's rule can change without changing SubmissionService or MarkingService.
class IPenaltyPolicy {
public:
    virtual ~IPenaltyPolicy() = default;
    // How many days late a submission is. 0 if on time.
    virtual int daysLate(TimePoint submittedAt, TimePoint deadline) const = 0;
    // The mark after the penalty.
    virtual double finalMark(double rawMark, int maxMark, int daysLate) const = 0;
};

}  // namespace lms
