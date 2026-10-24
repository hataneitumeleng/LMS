#pragma once
#include "services/IPenaltyPolicy.h"

namespace lms {

// The department's current rule: 10% of the maximum mark per day late,
// and a final mark of 0 after more than 3 days (BR-01, BR-02).
class TenPercentPerDayPolicy : public IPenaltyPolicy {
public:
    int daysLate(TimePoint submittedAt, TimePoint deadline) const override;
    double finalMark(double rawMark, int maxMark, int daysLate) const override;

private:
    static constexpr double kRatePerDay = 0.10;
    static constexpr int kMaxDays = 3;
};

}  // namespace lms
