// Service tests for ModerationService: approving, adjusting with a reason, releasing
// (FR-14, FR-15, BR-08, FR-17) and the HoD's department boundary (FR-02).
// Fixture: see ServiceTest.h.
#include "../fakes/ServiceTest.h"
#include "domain/Errors.h"

namespace lms::test {
namespace {

class ModerationServiceTest : public ServiceTest {
protected:
    // itu and neo submit on time; marks are given only when asked for.
    void SetUp() override {
        clock.set("2026-10-29 12:00:00");
        ituWork = submissionService.submit(itu, project1, makeFile("itu.txt")).id;
        neoWork = submissionService.submit(neo, project1, makeFile("neo.txt")).id;
        clock.set("2026-11-02 09:00:00");
    }
    void markBoth() {
        markingService.mark(thabo, ituWork, 70, "Good");
        markingService.mark(thabo, neoWork, 60, "Fair");
    }
    // A second department, Arts, with its own HoD zola and an assignment in ART101.
    int addArts() {
        const int arts = courses.saveDepartment(Department{0, "Arts"});
        zola = addUser(Role::HoD, "zola", arts);
        const int art101 = courses.saveCourse(Course{0, "ART101", "Drawing", arts});
        Assignment a;
        a.courseId = art101;
        a.title = "Sketch";
        a.deadline = at("2026-10-30 17:00:00");
        a.maxMark = 100;
        return assignments.save(a);
    }
    Submission stored(int submissionId) { return *submissions.findById(submissionId); }

    int ituWork = 0, neoWork = 0, zola = 0;
};

// ---- Who may moderate (FR-02) ----

TEST_F(ModerationServiceTest, HodSeesTheDepartmentsAssignments) {
    markingService.mark(thabo, ituWork, 70, "Good");
    const auto rows = moderationService.assignmentsInDepartment(mpho);
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].courseCode, "CS101");
    EXPECT_EQ(rows[0].assignment.title, "Project 1");
    EXPECT_EQ(rows[0].submissions, 2);
    EXPECT_EQ(rows[0].notModerated, 2);
}

TEST_F(ModerationServiceTest, OnlyAnHodCanModerate) {
    markBoth();
    EXPECT_THROW(moderationService.approveAll(thabo, project1), AuthError);
    EXPECT_THROW(moderationService.approveAll(admin, project1), AuthError);
    EXPECT_THROW(moderationService.assignmentsInDepartment(itu), AuthError);
}

TEST_F(ModerationServiceTest, DeactivatedHodIsRefused) {
    auto hod = users.findById(mpho);
    hod->setActive(false);
    users.save(*hod);
    EXPECT_THROW(moderationService.assignmentsInDepartment(mpho), AuthError);
}

TEST_F(ModerationServiceTest, HodOfAnotherDepartmentIsRefused) {  // BR-08, FR-02
    addArts();
    markBoth();
    EXPECT_THROW(moderationService.approveAll(zola, project1), AuthError);
    EXPECT_THROW(moderationService.adjust(zola, ituWork, 80, "Reason"), AuthError);
    EXPECT_THROW(moderationService.release(zola, project1), AuthError);
    EXPECT_THROW(moderationService.latestFile(zola, ituWork), AuthError);
}

TEST_F(ModerationServiceTest, EachHodSeesOnlyTheirOwnAssignments) {
    addArts();
    EXPECT_EQ(moderationService.assignmentsInDepartment(mpho).size(), 1u);
    const auto artsRows = moderationService.assignmentsInDepartment(zola);
    ASSERT_EQ(artsRows.size(), 1u);
    EXPECT_EQ(artsRows[0].courseCode, "ART101");
}

TEST_F(ModerationServiceTest, ListShowsOnlyMarkedWork) {
    markingService.mark(thabo, ituWork, 70, "Good");  // neo is not marked yet
    const auto list = moderationService.listForModeration(mpho, project1);
    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].id, ituWork);
}

// ---- Approve (FR-14) ----

TEST_F(ModerationServiceTest, ApproveAllKeepsTheMarks) {
    markBoth();
    EXPECT_EQ(moderationService.approveAll(mpho, project1), 2);
    const Submission s = stored(ituWork);
    EXPECT_EQ(s.status, SubmissionStatus::Moderated);
    EXPECT_DOUBLE_EQ(*s.finalMark, 70.0);
    EXPECT_EQ(s.moderatedBy, mpho);
    EXPECT_EQ(s.moderatedAt, at("2026-11-02 09:00:00"));
    EXPECT_EQ(moderationService.assignmentsInDepartment(mpho)[0].notModerated, 0);
}

TEST_F(ModerationServiceTest, ApproveAllWritesNoAuditEntry) {  // no mark changed (FR-17)
    markBoth();
    moderationService.approveAll(mpho, project1);
    EXPECT_EQ(auditLog.findAll().size(), 2u);  // just the two first marks
}

TEST_F(ModerationServiceTest, ApproveAllSkipsUnmarkedWork) {
    markingService.mark(thabo, ituWork, 70, "Good");
    EXPECT_EQ(moderationService.approveAll(mpho, project1), 1);
    EXPECT_TRUE(stored(neoWork).isOpen());
}

// ---- Adjust (FR-14, BR-08) ----

TEST_F(ModerationServiceTest, AdjustChangesTheMarkAndIsAudited) {
    markBoth();
    moderationService.adjust(mpho, neoWork, 65, "Marking scheme applied wrongly");
    const Submission s = stored(neoWork);
    EXPECT_EQ(s.status, SubmissionStatus::Moderated);
    EXPECT_DOUBLE_EQ(*s.finalMark, 65.0);
    const auto entries = auditLog.findAll();
    ASSERT_EQ(entries.size(), 3u);
    EXPECT_EQ(entries[2].userId, mpho);
    EXPECT_DOUBLE_EQ(*entries[2].oldValue, 60.0);
    EXPECT_DOUBLE_EQ(entries[2].newValue, 65.0);
    EXPECT_EQ(entries[2].reason, "Marking scheme applied wrongly");
}

TEST_F(ModerationServiceTest, AdjustNeedsAReason) {  // BR-08
    markBoth();
    EXPECT_THROW(moderationService.adjust(mpho, neoWork, 65, ""), ValidationError);
    EXPECT_THROW(moderationService.adjust(mpho, neoWork, 65, "  "), ValidationError);
    EXPECT_DOUBLE_EQ(*stored(neoWork).finalMark, 60.0);
}

TEST_F(ModerationServiceTest, AdjustedMarkMustBeInRange) {
    markBoth();
    EXPECT_THROW(moderationService.adjust(mpho, neoWork, -1, "Reason"), ValidationError);
    EXPECT_THROW(moderationService.adjust(mpho, neoWork, 101, "Reason"), ValidationError);
    EXPECT_NO_THROW(moderationService.adjust(mpho, neoWork, 100, "Reason"));
}

TEST_F(ModerationServiceTest, UnmarkedWorkCannotBeAdjusted) {
    EXPECT_THROW(moderationService.adjust(mpho, ituWork, 50, "Reason"), ValidationError);
    EXPECT_TRUE(auditLog.findAll().empty());
}

// ---- Release (FR-15) ----

TEST_F(ModerationServiceTest, ReleaseIsRefusedUntilEverythingIsModerated) {
    markBoth();
    try {
        moderationService.release(mpho, project1);
        FAIL() << "release should have been refused";
    } catch (const ValidationError& e) {
        EXPECT_STREQ(e.what(), "Cannot release: 2 submission(s) are not moderated yet");
    }
    EXPECT_EQ(stored(ituWork).status, SubmissionStatus::Marked);
}

TEST_F(ModerationServiceTest, UnmarkedWorkAlsoBlocksRelease) {
    markingService.mark(thabo, ituWork, 70, "Good");
    moderationService.approveAll(mpho, project1);
    EXPECT_THROW(moderationService.release(mpho, project1), ValidationError);  // neo
}

TEST_F(ModerationServiceTest, ReleaseShowsTheMarksToStudents) {
    markBoth();
    moderationService.approveAll(mpho, project1);
    EXPECT_EQ(moderationService.release(mpho, project1), 2);
    EXPECT_EQ(stored(ituWork).status, SubmissionStatus::Released);
    EXPECT_EQ(submissionService.listForStudent(itu)[0].status, "Released: 70.0 / 100");
}

TEST_F(ModerationServiceTest, NothingToReleaseIsReported) {
    const int sketch = addArts();
    EXPECT_THROW(moderationService.release(zola, sketch), ValidationError);
}

TEST_F(ModerationServiceTest, AdjustAfterReleaseKeepsItReleased) {  // BR-08
    markBoth();
    moderationService.approveAll(mpho, project1);
    moderationService.release(mpho, project1);
    moderationService.adjust(mpho, ituWork, 72, "Appeal upheld");
    EXPECT_EQ(stored(ituWork).status, SubmissionStatus::Released);
    EXPECT_EQ(submissionService.listForStudent(itu)[0].status, "Released: 72.0 / 100");
    EXPECT_EQ(auditLog.findAll().back().reason, "Appeal upheld");
}

TEST_F(ModerationServiceTest, HodCanOpenTheSubmittedFile) {
    markBoth();
    EXPECT_EQ(moderationService.latestFile(mpho, ituWork).versionNo, 1);
}

}  // namespace
}  // namespace lms::test
