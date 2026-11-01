// Unit tests for the late-penalty Strategy (BR-01, BR-02, FR-13).
// Boundary values: on time, 1 minute late, exactly 24 h, 24 h + 1 minute, 3 days, over 3 days.
#include "../fakes/FakeClock.h"
#include "services/TenPercentPerDayPolicy.h"

#include <gtest/gtest.h>

namespace lms::test {
namespace {

const TimePoint kDeadline = at("2026-10-30 17:00:00");

class PenaltyPolicyTest : public ::testing::Test {
protected:
    TenPercentPerDayPolicy policy;
};

// ---- daysLate (BR-02: every started 24-hour period counts) ----

TEST_F(PenaltyPolicyTest, BeforeDeadlineIsZeroDays) {
    EXPECT_EQ(policy.daysLate(at("2026-10-30 16:59:00"), kDeadline), 0);
}

TEST_F(PenaltyPolicyTest, ExactlyAtDeadlineIsZeroDays) {
    EXPECT_EQ(policy.daysLate(kDeadline, kDeadline), 0);
}

TEST_F(PenaltyPolicyTest, OneSecondLateIsOneDay) {
    EXPECT_EQ(policy.daysLate(at("2026-10-30 17:00:01"), kDeadline), 1);
}

TEST_F(PenaltyPolicyTest, Exactly24HoursLateIsStillOneDay) {
    EXPECT_EQ(policy.daysLate(at("2026-10-31 17:00:00"), kDeadline), 1);
}

TEST_F(PenaltyPolicyTest, OneMinuteOver24HoursIsTwoDays) {
    EXPECT_EQ(policy.daysLate(at("2026-10-31 17:01:00"), kDeadline), 2);
}

TEST_F(PenaltyPolicyTest, Exactly72HoursLateIsThreeDays) {
    EXPECT_EQ(policy.daysLate(at("2026-11-02 17:00:00"), kDeadline), 3);
}

TEST_F(PenaltyPolicyTest, OneMinuteOver72HoursIsFourDays) {
    EXPECT_EQ(policy.daysLate(at("2026-11-02 17:01:00"), kDeadline), 4);
}

// ---- finalMark (BR-01: 10% of the MAX mark per day, 0 after 3 days, never below 0) ----

TEST_F(PenaltyPolicyTest, OnTimeKeepsRawMark) {
    EXPECT_DOUBLE_EQ(policy.finalMark(60.0, 100, 0), 60.0);
}

TEST_F(PenaltyPolicyTest, OneDayLateLosesTenPercentOfMax) {
    EXPECT_DOUBLE_EQ(policy.finalMark(60.0, 100, 1), 50.0);
}

TEST_F(PenaltyPolicyTest, ThreeDaysLateLosesThirtyPercentOfMax) {
    EXPECT_DOUBLE_EQ(policy.finalMark(60.0, 100, 3), 30.0);
}

TEST_F(PenaltyPolicyTest, MoreThanThreeDaysLateGetsZero) {
    EXPECT_DOUBLE_EQ(policy.finalMark(100.0, 100, 4), 0.0);
}

TEST_F(PenaltyPolicyTest, PenaltyIsBasedOnMaxMarkNotRawMark) {
    // max 50: 10% of max is 5 marks per day, whatever the raw mark is
    EXPECT_DOUBLE_EQ(policy.finalMark(40.0, 50, 2), 30.0);
}

TEST_F(PenaltyPolicyTest, FinalMarkNeverGoesBelowZero) {
    EXPECT_DOUBLE_EQ(policy.finalMark(5.0, 100, 1), 0.0);
}

TEST_F(PenaltyPolicyTest, ZeroRawMarkStaysZero) {
    EXPECT_DOUBLE_EQ(policy.finalMark(0.0, 100, 2), 0.0);
}

// The two methods together, as SubmissionService and MarkingService use them.
TEST_F(PenaltyPolicyTest, SixteenHoursLateWithRawSixtyGivesFifty) {
    const int days = policy.daysLate(at("2026-10-31 09:00:00"), kDeadline);
    EXPECT_DOUBLE_EQ(policy.finalMark(60.0, 100, days), 50.0);
}

}  // namespace
}  // namespace lms::test
