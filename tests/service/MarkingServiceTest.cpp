// Service tests for MarkingService: marking with the late penalty (FR-12, FR-13), amending
// (FR-18), the audit entry for every mark change (FR-17) and who may mark (FR-02).
// Fixture: see ServiceTest.h.
#include "../fakes/ServiceTest.h"
#include "domain/Errors.h"

namespace lms::test {
namespace {

class MarkingServiceTest : public ServiceTest {
protected:
    // The student submits at the given time and the id of the submission is returned.
    int submitAt(const std::string& time, int student = 0) {
        clock.set(time);
        return submissionService
            .submit(student == 0 ? itu : student, project1, makeFile("essay.txt"))
            .id;
    }
    double markIt(int submissionId, double raw, const std::string& feedback = "Good work") {
        return markingService.mark(thabo, submissionId, raw, feedback);
    }
    Submission stored(int submissionId) { return *submissions.findById(submissionId); }
};

// ---- Marking and the late penalty (FR-12, FR-13, BR-02) ----

TEST_F(MarkingServiceTest, OnTimeWorkKeepsTheRawMark) {
    const int id = submitAt("2026-10-29 12:00:00");
    EXPECT_DOUBLE_EQ(markIt(id, 70, "Clear structure"), 70.0);
    const Submission s = stored(id);
    EXPECT_EQ(s.status, SubmissionStatus::Marked);
    EXPECT_DOUBLE_EQ(*s.rawMark, 70.0);
    EXPECT_DOUBLE_EQ(*s.finalMark, 70.0);
    EXPECT_EQ(s.feedback, "Clear structure");
    EXPECT_EQ(s.markedBy, thabo);
}

TEST_F(MarkingServiceTest, PenaltyIsAppliedAutomatically) {  // FR-13
    const int id = submitAt("2026-11-01 10:00:00");  // 2 days late
    EXPECT_DOUBLE_EQ(markIt(id, 70), 50.0);          // 70 - 2 x 10% of 100
    EXPECT_DOUBLE_EQ(*stored(id).rawMark, 70.0);     // the raw mark is kept too
}

TEST_F(MarkingServiceTest, MoreThanThreeDaysLateGivesZero) {  // BR-02
    const int id = submitAt("2026-11-03 18:00:00");           // 4 days late
    EXPECT_DOUBLE_EQ(markIt(id, 90), 0.0);
}

TEST_F(MarkingServiceTest, MarkMustBeFromZeroToTheMaximum) {  // boundary values
    const int first = submitAt("2026-10-29 12:00:00");
    const int second = submitAt("2026-10-29 12:00:00", neo);
    EXPECT_THROW(markIt(first, -1), ValidationError);
    EXPECT_THROW(markIt(first, 100.5), ValidationError);
    EXPECT_DOUBLE_EQ(markIt(first, 0), 0.0);
    EXPECT_DOUBLE_EQ(markIt(second, 100), 100.0);
}

TEST_F(MarkingServiceTest, FeedbackIsRequired) {  // A8
    const int id = submitAt("2026-10-29 12:00:00");
    EXPECT_THROW(markIt(id, 70, ""), ValidationError);
    EXPECT_THROW(markIt(id, 70, "   "), ValidationError);
    EXPECT_TRUE(stored(id).isOpen());  // nothing was saved
}

TEST_F(MarkingServiceTest, LecturerOfAnotherCourseCannotMark) {  // FR-02
    const int id = submitAt("2026-10-29 12:00:00");
    EXPECT_THROW(markingService.mark(lerato, id, 70, "Good"), AuthError);
    EXPECT_THROW(markingService.mark(mpho, id, 70, "Good"), AuthError);
}

TEST_F(MarkingServiceTest, UnknownSubmissionIsRefused) {
    EXPECT_THROW(markIt(99, 70), ValidationError);
}

TEST_F(MarkingServiceTest, MarkingTwiceIsRefused) {
    const int id = submitAt("2026-10-29 12:00:00");
    markIt(id, 70);
    EXPECT_THROW(markIt(id, 80), ValidationError);
}

// ---- The audit trail (FR-17) ----

TEST_F(MarkingServiceTest, FirstMarkIsWrittenToTheAuditTrail) {
    const int id = submitAt("2026-10-29 12:00:00");
    clock.set("2026-11-02 08:30:00");
    markIt(id, 70);
    const auto entries = auditLog.findAll();
    ASSERT_EQ(entries.size(), 1u);
    EXPECT_EQ(entries[0].submissionId, id);
    EXPECT_EQ(entries[0].userId, thabo);
    EXPECT_EQ(entries[0].changedAt, at("2026-11-02 08:30:00"));
    EXPECT_FALSE(entries[0].oldValue.has_value());  // no old value for a first mark
    EXPECT_DOUBLE_EQ(entries[0].newValue, 70.0);
    EXPECT_EQ(entries[0].reason, "First mark");
}

TEST_F(MarkingServiceTest, RefusedMarkWritesNoAuditEntry) {
    const int id = submitAt("2026-10-29 12:00:00");
    EXPECT_THROW(markIt(id, 101), ValidationError);
    EXPECT_TRUE(auditLog.findAll().empty());
}

// ---- Amending a mark (FR-18) ----

TEST_F(MarkingServiceTest, AmendChangesTheMarkAndIsAudited) {
    const int id = submitAt("2026-10-29 12:00:00");
    markIt(id, 70);
    EXPECT_DOUBLE_EQ(markingService.amend(thabo, id, 75, "Better", "Missed a section"), 75.0);
    EXPECT_DOUBLE_EQ(*stored(id).finalMark, 75.0);
    EXPECT_EQ(stored(id).feedback, "Better");
    const auto entries = auditLog.findAll();
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_DOUBLE_EQ(*entries[1].oldValue, 70.0);
    EXPECT_DOUBLE_EQ(entries[1].newValue, 75.0);
    EXPECT_EQ(entries[1].reason, "Missed a section");
}

TEST_F(MarkingServiceTest, AmendStillAppliesThePenalty) {
    const int id = submitAt("2026-10-31 10:00:00");  // 1 day late
    markIt(id, 70);                                  // 60
    EXPECT_DOUBLE_EQ(markingService.amend(thabo, id, 80, "Better", "Re-read"), 70.0);
}

TEST_F(MarkingServiceTest, AmendNeedsAReason) {
    const int id = submitAt("2026-10-29 12:00:00");
    markIt(id, 70);
    EXPECT_THROW(markingService.amend(thabo, id, 75, "Better", " "), ValidationError);
    EXPECT_DOUBLE_EQ(*stored(id).finalMark, 70.0);
}

TEST_F(MarkingServiceTest, AmendBeforeMarkingIsRefused) {
    const int id = submitAt("2026-10-29 12:00:00");
    EXPECT_THROW(markingService.amend(thabo, id, 75, "Better", "Reason"), ValidationError);
}

TEST_F(MarkingServiceTest, OnlyTheLecturerWhoMarkedCanAmend) {
    courses.addLecturer(cs101, lerato);  // a second lecturer of CS101
    const int id = submitAt("2026-10-29 12:00:00");
    markIt(id, 70);
    EXPECT_THROW(markingService.amend(lerato, id, 75, "Better", "Reason"), AuthError);
}

TEST_F(MarkingServiceTest, AmendAfterModerationIsRefused) {  // only the HoD can change it now
    const int id = submitAt("2026-10-29 12:00:00");
    markIt(id, 70);
    moderationService.approveAll(mpho, project1);
    EXPECT_THROW(markingService.amend(thabo, id, 75, "Better", "Reason"), ValidationError);
}

// ---- Lists and files ----

TEST_F(MarkingServiceTest, ListForMarkingShowsEverySubmission) {
    submitAt("2026-10-29 12:00:00");
    submitAt("2026-10-29 13:00:00", neo);
    EXPECT_EQ(markingService.listForMarking(thabo, project1).size(), 2u);
    EXPECT_THROW(markingService.listForMarking(lerato, project1), AuthError);
}

TEST_F(MarkingServiceTest, LatestFileIsTheNewestVersion) {  // BR-05
    const int id = submitAt("2026-10-28 09:00:00");
    submitAt("2026-10-29 09:00:00");  // version 2
    EXPECT_EQ(markingService.latestFile(thabo, id).versionNo, 2);
    EXPECT_THROW(markingService.latestFile(lerato, id), AuthError);
}

}  // namespace
}  // namespace lms::test
