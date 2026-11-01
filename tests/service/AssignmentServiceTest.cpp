// Service tests for AssignmentService: creating assignments (FR-07), extensions (FR-08,
// BR-03, BR-04) and assignment briefs (FR-24). Fixture: see ServiceTest.h.
#include "../fakes/ServiceTest.h"
#include "domain/Errors.h"

#include <chrono>
#include <filesystem>

namespace lms::test {
namespace {

using std::chrono::hours;
using std::chrono::seconds;

const TimePoint kDue = at("2026-10-30 17:00:00");  // Project 1's normal deadline

class AssignmentServiceTest : public ServiceTest {
protected:
    int create(int lecturer, const std::string& title, TimePoint deadline, int maxMark) {
        return assignmentService.createAssignment(lecturer, cs101, title, deadline, maxMark);
    }
    void extend(int student, TimePoint newDeadline, const std::string& reason = "Medical") {
        assignmentService.grantExtension(thabo, project1, student, newDeadline, reason);
    }
};

// ---- Creating assignments (FR-07) ----

TEST_F(AssignmentServiceTest, LecturerOfTheCourseCanCreate) {
    const int id = create(thabo, "Project 2", at("2026-11-20 17:00:00"), 50);
    const auto saved = assignments.find(id);
    ASSERT_TRUE(saved);
    EXPECT_EQ(saved->title, "Project 2");
    EXPECT_EQ(saved->maxMark, 50);
    EXPECT_EQ(saved->createdBy, thabo);
}

TEST_F(AssignmentServiceTest, LecturerOfAnotherCourseIsRefused) {  // FR-02
    EXPECT_THROW(create(lerato, "Project 2", kDue, 100), AuthError);
}

TEST_F(AssignmentServiceTest, StudentCannotCreate) {
    EXPECT_THROW(create(itu, "Project 2", kDue, 100), AuthError);
}

TEST_F(AssignmentServiceTest, DeactivatedLecturerIsRefused) {
    auto user = users.findById(thabo);
    user->setActive(false);
    users.save(*user);
    EXPECT_THROW(create(thabo, "Project 2", kDue, 100), AuthError);
}

TEST_F(AssignmentServiceTest, UnknownCourseIsRefused) {
    EXPECT_THROW(assignmentService.createAssignment(thabo, 99, "X", kDue, 100), ValidationError);
}

TEST_F(AssignmentServiceTest, EmptyOrBlankTitleIsRefused) {
    EXPECT_THROW(create(thabo, "", kDue, 100), ValidationError);
    EXPECT_THROW(create(thabo, "   ", kDue, 100), ValidationError);
}

TEST_F(AssignmentServiceTest, MaxMarkBoundaries) {  // 1 to 100, also a CHECK in the database
    EXPECT_THROW(create(thabo, "A", kDue, 0), ValidationError);
    EXPECT_NO_THROW(create(thabo, "B", kDue, 1));
    EXPECT_NO_THROW(create(thabo, "C", kDue, 100));
    EXPECT_THROW(create(thabo, "D", kDue, 101), ValidationError);
}

TEST_F(AssignmentServiceTest, DeadlineMustBeInTheFuture) {
    EXPECT_THROW(create(thabo, "A", clock.now(), 100), ValidationError);  // exactly now
    EXPECT_THROW(create(thabo, "B", clock.now() - hours(1), 100), ValidationError);
    EXPECT_NO_THROW(create(thabo, "C", clock.now() + seconds(1), 100));
}

TEST_F(AssignmentServiceTest, AssignmentsForListsOnlyForTheLecturer) {
    EXPECT_EQ(assignmentService.assignmentsFor(thabo, cs101).size(), 1u);
    EXPECT_THROW(assignmentService.assignmentsFor(lerato, cs101), AuthError);
}

TEST_F(AssignmentServiceTest, CoursesTaughtByListsTheLecturersCourses) {
    EXPECT_EQ(assignmentService.coursesTaughtBy(thabo).size(), 1u);
    EXPECT_TRUE(assignmentService.coursesTaughtBy(lerato).empty());
}

// ---- Extensions (FR-08, BR-03) ----

TEST_F(AssignmentServiceTest, ExtensionChangesThatStudentsDeadlineOnly) {
    extend(itu, kDue + hours(72));
    const auto ext = assignments.findExtension(project1, itu);
    ASSERT_TRUE(ext);
    EXPECT_EQ(ext->newDeadline, kDue + hours(72));
    EXPECT_EQ(ext->reason, "Medical");
    EXPECT_EQ(ext->grantedBy, thabo);
    EXPECT_FALSE(assignments.findExtension(project1, neo));
}

TEST_F(AssignmentServiceTest, ExactlyFourteenDaysIsAllowed) {
    EXPECT_NO_THROW(extend(itu, kDue + hours(24 * 14)));
}

TEST_F(AssignmentServiceTest, OneSecondOverFourteenDaysIsRefused) {
    EXPECT_THROW(extend(itu, kDue + hours(24 * 14) + seconds(1)), ValidationError);
}

TEST_F(AssignmentServiceTest, NewDeadlineMustBeLaterThanTheNormalOne) {
    EXPECT_THROW(extend(itu, kDue), ValidationError);
    EXPECT_THROW(extend(itu, kDue - hours(1)), ValidationError);
}

TEST_F(AssignmentServiceTest, ExtensionNeedsAReason) {
    EXPECT_THROW(extend(itu, kDue + hours(24), ""), ValidationError);
    EXPECT_THROW(extend(itu, kDue + hours(24), "  "), ValidationError);
}

TEST_F(AssignmentServiceTest, StudentNotEnrolledIsRefused) {
    EXPECT_THROW(extend(sipho, kDue + hours(24)), ValidationError);
}

TEST_F(AssignmentServiceTest, OnlyTheCoursesLecturerCanGrant) {
    EXPECT_THROW(assignmentService.grantExtension(lerato, project1, itu, kDue + hours(24), "X"),
                 AuthError);
}

TEST_F(AssignmentServiceTest, SecondExtensionReplacesTheFirst) {
    extend(itu, kDue + hours(24));
    extend(itu, kDue + hours(48), "Second certificate");
    EXPECT_EQ(assignments.findExtension(project1, itu)->newDeadline, kDue + hours(48));
}

TEST_F(AssignmentServiceTest, NoExtensionOnceTheWorkIsMarked) {  // BR-03
    clock.set("2026-10-29 10:00:00");
    const Submission s = submissionService.submit(itu, project1, makeFile("essay.txt"));
    auto stored = *submissions.findById(s.id);
    stored.recordMark(70, 70, "Good", thabo);
    submissions.save(stored);
    EXPECT_THROW(extend(itu, kDue + hours(24)), ValidationError);
}

// ---- BR-04: days late are worked out again for work already submitted ----

TEST_F(AssignmentServiceTest, ExtensionCoveringTheLatenessMakesItOnTime) {
    clock.set("2026-10-31 09:00:00");  // 16 hours late
    const Submission s = submissionService.submit(neo, project1, makeFile("neo.txt"));
    ASSERT_EQ(s.status, SubmissionStatus::Late);
    extend(neo, at("2026-11-01 17:00:00"));
    const auto after = submissions.findById(s.id);
    EXPECT_EQ(after->status, SubmissionStatus::Submitted);
    EXPECT_EQ(after->daysLate, 0);
}

TEST_F(AssignmentServiceTest, ExtensionNotCoveringAllTheLatenessLeavesFewerDays) {
    clock.set("2026-11-02 10:00:00");  // 3 days late against 30 Oct
    const Submission s = submissionService.submit(neo, project1, makeFile("neo.txt"));
    ASSERT_EQ(s.daysLate, 3);
    extend(neo, at("2026-11-01 17:00:00"));  // now 17 hours late
    const auto after = submissions.findById(s.id);
    EXPECT_EQ(after->status, SubmissionStatus::Late);
    EXPECT_EQ(after->daysLate, 1);
}

// ---- Assignment briefs (FR-24) ----

TEST_F(AssignmentServiceTest, AttachBriefCopiesTheFile) {
    assignmentService.attachBrief(thabo, project1, makeFile("brief.pdf", "Instructions"));
    const std::string stored = assignments.find(project1)->briefPath;
    EXPECT_NE(stored.find("brief" + std::to_string(project1) + "_1.pdf"), std::string::npos);
    EXPECT_TRUE(std::filesystem::exists(stored));
}

TEST_F(AssignmentServiceTest, ReplacingABriefDeletesTheOldFile) {
    assignmentService.attachBrief(thabo, project1, makeFile("brief.pdf", "v1"));
    const std::string first = assignments.find(project1)->briefPath;
    assignmentService.attachBrief(thabo, project1, makeFile("brief2.pdf", "v2"));
    const std::string second = assignments.find(project1)->briefPath;
    EXPECT_NE(first, second);
    EXPECT_FALSE(std::filesystem::exists(first));
    EXPECT_TRUE(std::filesystem::exists(second));
}

TEST_F(AssignmentServiceTest, BriefFromAnotherLecturerIsRefused) {
    EXPECT_THROW(assignmentService.attachBrief(lerato, project1, makeFile("b.pdf")), AuthError);
}

TEST_F(AssignmentServiceTest, MissingBriefFileIsRefused) {
    EXPECT_THROW(assignmentService.attachBrief(thabo, project1, "no/such/file.pdf"),
                 ValidationError);
}

TEST_F(AssignmentServiceTest, FailedSaveDeletesTheCopiedBrief) {  // like Figure 6
    assignments.failNextSave = true;
    EXPECT_ANY_THROW(assignmentService.attachBrief(thabo, project1, makeFile("b.pdf")));
    EXPECT_EQ(filesStored(), 0);
    EXPECT_TRUE(assignments.find(project1)->briefPath.empty());
}

}  // namespace
}  // namespace lms::test
