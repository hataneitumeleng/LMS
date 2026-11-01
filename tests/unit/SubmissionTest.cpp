// Unit tests for the Submission state machine (Week 3 Figure 4; BR-05, BR-06, BR-08,
// FR-12, FR-14, FR-15): Submitted/Late -> Marked -> Moderated -> Released.
#include "../fakes/FakeClock.h"
#include "domain/Errors.h"
#include "domain/Submission.h"

#include <gtest/gtest.h>

namespace lms::test {
namespace {

const TimePoint kDeadline = at("2026-10-30 17:00:00");
const TimePoint kNow = at("2026-11-03 10:00:00");

Submission markedSubmission() {
    Submission s;
    s.recordMark(70.0, 70.0, "Good", 3);
    return s;
}

// ---- Submitted and Late ----

TEST(SubmissionTest, NewSubmissionIsOpenAndSubmitted) {
    const Submission s;
    EXPECT_TRUE(s.isOpen());
    EXPECT_EQ(s.status, SubmissionStatus::Submitted);
}

TEST(SubmissionTest, ZeroDaysLateMeansSubmitted) {
    Submission s;
    s.setLateness(0);
    EXPECT_EQ(s.status, SubmissionStatus::Submitted);
    EXPECT_EQ(s.daysLate, 0);
}

TEST(SubmissionTest, OneOrMoreDaysLateMeansLate) {
    Submission s;
    s.setLateness(2);
    EXPECT_EQ(s.status, SubmissionStatus::Late);
    EXPECT_EQ(s.daysLate, 2);
}

TEST(SubmissionTest, AnExtensionCanTurnLateBackIntoSubmitted) {
    Submission s;
    s.setLateness(1);
    s.setLateness(0);  // BR-04: days late worked out again after an extension
    EXPECT_EQ(s.status, SubmissionStatus::Submitted);
}

TEST(SubmissionTest, LatenessCannotChangeAfterMarking) {
    Submission s = markedSubmission();
    EXPECT_THROW(s.setLateness(1), ValidationError);
}

// ---- Resubmission (BR-05, BR-06) ----

TEST(SubmissionTest, CanResubmitBeforeDeadline) {
    const Submission s;
    EXPECT_TRUE(s.canResubmit(at("2026-10-30 16:00:00"), kDeadline));
}

TEST(SubmissionTest, CanResubmitExactlyAtDeadline) {
    const Submission s;
    EXPECT_TRUE(s.canResubmit(kDeadline, kDeadline));
}

TEST(SubmissionTest, CannotResubmitAfterDeadline) {
    const Submission s;
    EXPECT_FALSE(s.canResubmit(at("2026-10-30 17:00:01"), kDeadline));
}

TEST(SubmissionTest, CannotResubmitOnceMarked) {
    const Submission s = markedSubmission();
    EXPECT_FALSE(s.canResubmit(at("2026-10-29 10:00:00"), kDeadline));
}

// ---- Marking (FR-12, FR-18) ----

TEST(SubmissionTest, RecordMarkStoresMarksAndSetsMarked) {
    Submission s;
    s.setLateness(1);
    s.recordMark(60.0, 50.0, "Late but good", 3);
    EXPECT_EQ(s.status, SubmissionStatus::Marked);
    EXPECT_DOUBLE_EQ(*s.rawMark, 60.0);
    EXPECT_DOUBLE_EQ(*s.finalMark, 50.0);
    EXPECT_EQ(s.feedback, "Late but good");
    EXPECT_EQ(*s.markedBy, 3);
    EXPECT_FALSE(s.isOpen());
}

TEST(SubmissionTest, MarkedWorkCanBeMarkedAgain) {  // the lecturer amends (FR-18)
    Submission s = markedSubmission();
    s.recordMark(75.0, 75.0, "Re-checked", 3);
    EXPECT_DOUBLE_EQ(*s.finalMark, 75.0);
    EXPECT_EQ(s.status, SubmissionStatus::Marked);
}

TEST(SubmissionTest, ModeratedWorkCannotBeMarkedByTheLecturer) {
    Submission s = markedSubmission();
    s.moderate(70.0, 2, kNow);
    EXPECT_THROW(s.recordMark(80.0, 80.0, "Change", 3), ValidationError);
}

// ---- Moderation (FR-14, BR-08) ----

TEST(SubmissionTest, UnmarkedWorkCannotBeModerated) {
    Submission s;
    EXPECT_THROW(s.moderate(50.0, 2, kNow), ValidationError);
}

TEST(SubmissionTest, ModerateSetsFinalMarkAndModerated) {
    Submission s = markedSubmission();
    s.moderate(65.0, 2, kNow);
    EXPECT_EQ(s.status, SubmissionStatus::Moderated);
    EXPECT_DOUBLE_EQ(*s.finalMark, 65.0);
    EXPECT_DOUBLE_EQ(*s.rawMark, 70.0);  // the lecturer's raw mark is kept
    EXPECT_EQ(*s.moderatedBy, 2);
    EXPECT_EQ(*s.moderatedAt, kNow);
}

TEST(SubmissionTest, HodCanAdjustAReleasedMarkAndItStaysReleased) {  // BR-08
    Submission s = markedSubmission();
    s.moderate(70.0, 2, kNow);
    s.release();
    s.moderate(68.0, 2, kNow);
    EXPECT_EQ(s.status, SubmissionStatus::Released);
    EXPECT_DOUBLE_EQ(*s.finalMark, 68.0);
}

// ---- Release (FR-15, BR-07) ----

TEST(SubmissionTest, ModeratedWorkCanBeReleased) {
    Submission s = markedSubmission();
    s.moderate(70.0, 2, kNow);
    s.release();
    EXPECT_EQ(s.status, SubmissionStatus::Released);
}

TEST(SubmissionTest, MarkedButNotModeratedCannotBeReleased) {
    Submission s = markedSubmission();
    EXPECT_THROW(s.release(), ValidationError);
}

TEST(SubmissionTest, OpenWorkCannotBeReleased) {
    Submission s;
    EXPECT_THROW(s.release(), ValidationError);
}

}  // namespace
}  // namespace lms::test
