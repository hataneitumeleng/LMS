// Service tests for SubmissionService: submitting and resubmitting (FR-10, FR-11, BR-05,
// BR-06), the student's list (FR-09, BR-07, FR-16) and assignment briefs (FR-24).
// Fixture: see ServiceTest.h.
#include "../fakes/ServiceTest.h"
#include "domain/Errors.h"

#include <filesystem>
#include <fstream>

namespace lms::test {
namespace {

class SubmissionServiceTest : public ServiceTest {
protected:
    Submission submitAt(const std::string& time, int student = 0) {
        clock.set(time);
        return submissionService.submit(student == 0 ? itu : student, project1,
                                        makeFile("essay.txt"));
    }
    // Moves a submission on, as the lecturer and HoD would, without their services.
    void markIt(int submissionId, double mark, const std::string& feedback = "Good work") {
        Submission s = *submissions.findById(submissionId);
        s.recordMark(mark, mark, feedback, thabo);
        submissions.save(s);
    }
    void moderateIt(int submissionId) {
        Submission s = *submissions.findById(submissionId);
        s.moderate(*s.finalMark, 99, clock.now());
        submissions.save(s);
    }
    void releaseIt(int submissionId) {
        Submission s = *submissions.findById(submissionId);
        s.release();
        submissions.save(s);
    }
    StudentAssignmentRow rowFor(int student) {
        const auto rows = submissionService.listForStudent(student);
        EXPECT_EQ(rows.size(), 1u);
        return rows.at(0);
    }
};

// ---- First submission (FR-10) ----

TEST_F(SubmissionServiceTest, OnTimeSubmissionIsSubmittedVersion1) {
    const Submission s = submitAt("2026-10-29 12:00:00");
    EXPECT_EQ(s.status, SubmissionStatus::Submitted);
    EXPECT_EQ(s.daysLate, 0);
    const auto version = submissions.latestVersion(s.id);
    ASSERT_TRUE(version);
    EXPECT_EQ(version->versionNo, 1);
    EXPECT_EQ(version->submittedAt, at("2026-10-29 12:00:00"));
}

TEST_F(SubmissionServiceTest, FileIsCopiedIntoTheStore) {
    const Submission s = submitAt("2026-10-29 12:00:00");
    const std::string stored = submissions.latestVersion(s.id)->filePath;
    EXPECT_TRUE(std::filesystem::exists(stored));
    EXPECT_NE(stored.find("sub" + std::to_string(s.id) + "_v1"), std::string::npos);
    EXPECT_EQ(filesStored(), 1);
}

TEST_F(SubmissionServiceTest, SubmittingExactlyAtTheDeadlineIsOnTime) {  // BR-01
    EXPECT_EQ(submitAt("2026-10-30 17:00:00").status, SubmissionStatus::Submitted);
}

TEST_F(SubmissionServiceTest, LateFirstSubmissionIsAcceptedAsLate) {  // BR-02, BR-06
    const Submission s = submitAt("2026-11-01 10:00:00");  // 1 day 17 hours late
    EXPECT_EQ(s.status, SubmissionStatus::Late);
    EXPECT_EQ(s.daysLate, 2);
}

TEST_F(SubmissionServiceTest, NotEnrolledStudentIsRefused) {  // FR-02
    EXPECT_THROW(submitAt("2026-10-29 12:00:00", sipho), AuthError);
    EXPECT_EQ(filesStored(), 0);
}

TEST_F(SubmissionServiceTest, UnknownAssignmentIsRefused) {
    EXPECT_THROW(submissionService.submit(itu, 99, makeFile("essay.txt")), ValidationError);
}

TEST_F(SubmissionServiceTest, MissingFileIsRefused) {
    EXPECT_THROW(submissionService.submit(itu, project1, "C:/no/such/file.pdf"),
                 ValidationError);
}

TEST_F(SubmissionServiceTest, EmptyFileIsRefused) {
    EXPECT_THROW(submissionService.submit(itu, project1, makeFile("empty.txt", "")),
                 ValidationError);
}

TEST_F(SubmissionServiceTest, ExtensionDeadlineIsUsed) {  // BR-03
    assignmentService.grantExtension(thabo, project1, itu, at("2026-11-03 17:00:00"), "Medical");
    EXPECT_EQ(submitAt("2026-11-02 12:00:00").status, SubmissionStatus::Submitted);
    // neo has no extension, so the same time is late for neo
    EXPECT_EQ(submitAt("2026-11-02 12:00:00", neo).status, SubmissionStatus::Late);
}

TEST_F(SubmissionServiceTest, DatabaseErrorRemovesTheCopiedFile) {  // Figure 6 note, NFR-07
    clock.set("2026-10-29 12:00:00");
    submissions.failNextAddVersion = true;
    EXPECT_THROW(submissionService.submit(itu, project1, makeFile("essay.txt")),
                 std::runtime_error);
    EXPECT_EQ(filesStored(), 0);
}

// ---- Resubmission (FR-11, BR-05, BR-06) ----

TEST_F(SubmissionServiceTest, ResubmitBeforeDeadlineAddsVersion2AndKeepsVersion1) {
    const Submission first = submitAt("2026-10-28 09:00:00");
    const Submission second = submitAt("2026-10-30 16:59:59");
    EXPECT_EQ(second.id, first.id);  // still one submission
    const auto versions = submissions.versions(first.id);
    ASSERT_EQ(versions.size(), 2u);
    EXPECT_EQ(versions[0].versionNo, 1);
    EXPECT_EQ(versions[1].versionNo, 2);
    EXPECT_EQ(filesStored(), 2);  // both files are kept
}

TEST_F(SubmissionServiceTest, ResubmitAfterDeadlineIsRefused) {  // BR-06
    submitAt("2026-10-28 09:00:00");
    EXPECT_THROW(submitAt("2026-10-30 17:00:01"), ValidationError);
    EXPECT_EQ(filesStored(), 1);
}

TEST_F(SubmissionServiceTest, ResubmitAfterMarkingIsRefused) {  // BR-06
    const Submission s = submitAt("2026-10-28 09:00:00");
    markIt(s.id, 70);
    EXPECT_THROW(submitAt("2026-10-29 09:00:00"), ValidationError);
}

TEST_F(SubmissionServiceTest, ResubmitUsesTheExtendedDeadline) {  // BR-03
    submitAt("2026-10-28 09:00:00");
    assignmentService.grantExtension(thabo, project1, itu, at("2026-11-03 17:00:00"), "Medical");
    const Submission s = submitAt("2026-11-02 09:00:00");
    EXPECT_EQ(s.status, SubmissionStatus::Submitted);
    EXPECT_EQ(submissions.versions(s.id).size(), 2u);
}

TEST_F(SubmissionServiceTest, DatabaseErrorOnResubmitRemovesOnlyTheNewFile) {
    submitAt("2026-10-28 09:00:00");
    submissions.failNextAddVersion = true;
    EXPECT_THROW(submitAt("2026-10-29 09:00:00"), std::runtime_error);
    EXPECT_EQ(filesStored(), 1);  // version 1 is still there
}

// ---- The student's list (FR-09, BR-07, FR-16) ----

TEST_F(SubmissionServiceTest, ListShowsNotSubmitted) {
    const StudentAssignmentRow row = rowFor(itu);
    EXPECT_EQ(row.courseCode, "CS101");
    EXPECT_EQ(row.title, "Project 1");
    EXPECT_EQ(row.status, "Not submitted");
    EXPECT_EQ(row.effectiveDeadline, at("2026-10-30 17:00:00"));
    EXPECT_FALSE(row.hasBrief);
}

TEST_F(SubmissionServiceTest, ListShowsLateDays) {
    submitAt("2026-10-31 18:00:00");
    EXPECT_EQ(rowFor(itu).status, "Late (2 days)");
}

TEST_F(SubmissionServiceTest, ListShowsTheExtendedDeadline) {
    assignmentService.grantExtension(thabo, project1, itu, at("2026-11-03 17:00:00"), "Medical");
    EXPECT_EQ(rowFor(itu).effectiveDeadline, at("2026-11-03 17:00:00"));
    EXPECT_EQ(rowFor(neo).effectiveDeadline, at("2026-10-30 17:00:00"));
}

TEST_F(SubmissionServiceTest, MarkStaysHiddenUntilReleased) {  // BR-07
    const Submission s = submitAt("2026-10-28 09:00:00");
    markIt(s.id, 70, "Good structure");
    EXPECT_EQ(rowFor(itu).status, "Being marked");
    EXPECT_EQ(rowFor(itu).feedback, "");
    moderateIt(s.id);
    EXPECT_EQ(rowFor(itu).status, "Being marked");
    EXPECT_EQ(rowFor(itu).feedback, "");
}

TEST_F(SubmissionServiceTest, ReleasedMarkAndFeedbackAreShown) {  // FR-16
    const Submission s = submitAt("2026-10-28 09:00:00");
    markIt(s.id, 70, "Good structure");
    moderateIt(s.id);
    releaseIt(s.id);
    EXPECT_EQ(rowFor(itu).status, "Released: 70.0 / 100");
    EXPECT_EQ(rowFor(itu).feedback, "Good structure");
}

TEST_F(SubmissionServiceTest, NotEnrolledStudentSeesNothing) {
    EXPECT_TRUE(submissionService.listForStudent(sipho).empty());
}

// ---- Assignment brief (FR-24) ----

TEST_F(SubmissionServiceTest, EnrolledStudentGetsTheBrief) {
    assignmentService.attachBrief(thabo, project1, makeFile("brief.pdf", "Do this"));
    const std::string path = submissionService.briefFor(itu, project1);
    EXPECT_TRUE(std::filesystem::exists(path));
    EXPECT_TRUE(rowFor(itu).hasBrief);
}

TEST_F(SubmissionServiceTest, NotEnrolledStudentCannotGetTheBrief) {  // FR-02
    assignmentService.attachBrief(thabo, project1, makeFile("brief.pdf", "Do this"));
    EXPECT_THROW(submissionService.briefFor(sipho, project1), AuthError);
}

TEST_F(SubmissionServiceTest, NoBriefAttachedIsReported) {
    EXPECT_THROW(submissionService.briefFor(itu, project1), ValidationError);
}

// ---- Start-up clean-up (Week 3, Section 4.3) ----

TEST_F(SubmissionServiceTest, RemoveOrphanFilesKeepsKnownFilesOnly) {
    const Submission s = submitAt("2026-10-28 09:00:00");
    assignmentService.attachBrief(thabo, project1, makeFile("brief.pdf", "Do this"));
    // A file left behind by a crash between copying it and committing (no row knows it)
    const std::filesystem::path stored = submissions.latestVersion(s.id)->filePath;
    std::ofstream(stored.parent_path() / "sub77_v1.txt") << "left over\n";
    ASSERT_EQ(filesStored(), 3);  // submission, brief and the stray file

    EXPECT_EQ(submissionService.removeOrphanFiles(), 1);
    EXPECT_EQ(filesStored(), 2);
    EXPECT_TRUE(std::filesystem::exists(stored));
    EXPECT_TRUE(std::filesystem::exists(submissionService.briefFor(itu, project1)));
}

TEST_F(SubmissionServiceTest, ClassWideExtensionRecalculatesLateSubmission) {
    // Submit one day after the original deadline.
    const Submission original = submitAt("2026-10-31 12:00:00");

    ASSERT_EQ(original.status, SubmissionStatus::Late);
    EXPECT_GT(original.daysLate, 0);

    // Extend the deadline for the entire class.
    const TimePoint newDeadline = at("2026-11-03 17:00:00");

    assignmentService.extendClassDeadline(
        thabo, project1, newDeadline, "Class-wide extension");

    // Reload the submission to check the stored changes.
    const auto updated = submissions.findById(original.id);

    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, SubmissionStatus::Submitted);
    EXPECT_EQ(updated->daysLate, 0);

    // The submission file history must remain intact.
    const auto version = submissions.latestVersion(original.id);
    ASSERT_TRUE(version.has_value());
    EXPECT_EQ(version->versionNo, 1);
}

TEST_F(SubmissionServiceTest, ClassWideExtensionPreservesMarkedSubmission) {
    // Submit after the original deadline.
    const Submission original = submitAt("2026-10-31 12:00:00");

    ASSERT_EQ(original.status, SubmissionStatus::Late);

    // Mark the submission before extending the deadline.
    markIt(original.id, 75.0, "Good work");

    const auto before = submissions.findById(original.id);
    ASSERT_TRUE(before.has_value());
    ASSERT_EQ(before->status, SubmissionStatus::Marked);

    // Extend the class deadline.
    assignmentService.extendClassDeadline(
        thabo,
        project1,
        at("2026-11-03 17:00:00"),
        "Class-wide extension");

    // Reload the marked submission.
    const auto after = submissions.findById(original.id);
    ASSERT_TRUE(after.has_value());

    // Existing marking information must remain unchanged.
    EXPECT_EQ(after->status, SubmissionStatus::Marked);
    EXPECT_EQ(after->rawMark, before->rawMark);
    EXPECT_EQ(after->finalMark, before->finalMark);
    EXPECT_EQ(after->feedback, before->feedback);
    EXPECT_EQ(after->daysLate, before->daysLate);
}

}  // namespace
}  // namespace lms::test
