#include "services/TenPercentPerDayPolicy.h"

#include <algorithm>
#include <chrono>

namespace lms {

int TenPercentPerDayPolicy::daysLate(TimePoint submittedAt, TimePoint deadline) const {
    if (submittedAt <= deadline) return 0;
    // BR-02: count every STARTED 24-hour period, so 1 minute late is 1 day
    // and exactly 24 hours late is still 1 day.
    const long long secondsLate =
        std::chrono::duration_cast<std::chrono::seconds>(submittedAt - deadline).count();
    const long long secondsPerDay = 24LL * 60 * 60;
    return static_cast<int>((secondsLate + secondsPerDay - 1) / secondsPerDay);
}

double TenPercentPerDayPolicy::finalMark(double rawMark, int maxMark, int daysLate) const {
    if (daysLate <= 0) return rawMark;
    if (daysLate > kMaxDays) return 0.0;  // BR-01: more than 3 days late gets 0
    // BR-01: 10% of max mark per day late, never below 0
    return std::max(0.0, rawMark - kRatePerDay * daysLate * maxMark);
}

}  // namespace lms
