// Service tests for AuditService: who may read the audit trail (FR-17, FR-02).
// Fixture: see ServiceTest.h.
#include "../fakes/ServiceTest.h"
#include "domain/Errors.h"

namespace lms::test {
namespace {

class AuditServiceTest : public ServiceTest {
protected:
    // One mark change in Science (CS101) and one in Arts (ART101, HoD zola).
    void SetUp() override {
        clock.set("2026-10-29 12:00:00");
        scienceWork = submissionService.submit(itu, project1, makeFile("itu.txt")).id;
        clock.set("2026-11-02 09:00:00");
        markingService.mark(thabo, scienceWork, 70, "Good");

        const int arts = courses.saveDepartment(Department{0, "Arts"});
        zola = addUser(Role::HoD, "zola", arts);
        const int art101 = courses.saveCourse(Course{0, "ART101", "Drawing", arts});
        Assignment sketch;
        sketch.courseId = art101;
        sketch.title = "Sketch";
        sketch.maxMark = 100;
        Submission work;
        work.assignmentId = assignments.save(sketch);
        work.studentId = sipho;
        artsWork = submissions.save(work);
        auditService.record(zola, artsWork, std::nullopt, 55, "First mark");
    }
    std::vector<AuditEntry> entriesFor(int userId) {
        return auditService.entriesFor(*users.findById(userId));
    }

    int scienceWork = 0, artsWork = 0, zola = 0;
};

TEST_F(AuditServiceTest, RecordStoresWhoWhenOldNewAndWhy) {
    clock.set("2026-11-03 10:15:00");
    auditService.record(mpho, scienceWork, 70.0, 68.0, "Rounded wrongly");
    const AuditEntry e = auditLog.findAll().back();
    EXPECT_EQ(e.userId, mpho);
    EXPECT_EQ(e.submissionId, scienceWork);
    EXPECT_EQ(e.changedAt, at("2026-11-03 10:15:00"));  // the time comes from IClock
    EXPECT_DOUBLE_EQ(*e.oldValue, 70.0);
    EXPECT_DOUBLE_EQ(e.newValue, 68.0);
    EXPECT_EQ(e.reason, "Rounded wrongly");
}

TEST_F(AuditServiceTest, AdministratorSeesEverything) {
    EXPECT_EQ(entriesFor(admin).size(), 2u);
}

TEST_F(AuditServiceTest, HodSeesOnlyTheirOwnDepartment) {
    const auto science = entriesFor(mpho);
    ASSERT_EQ(science.size(), 1u);
    EXPECT_EQ(science[0].submissionId, scienceWork);
    const auto arts = entriesFor(zola);
    ASSERT_EQ(arts.size(), 1u);
    EXPECT_EQ(arts[0].submissionId, artsWork);
}

TEST_F(AuditServiceTest, LecturersAndStudentsCannotSeeTheTrail) {
    EXPECT_THROW(entriesFor(thabo), AuthError);
    EXPECT_THROW(entriesFor(itu), AuthError);
}

}  // namespace
}  // namespace lms::test
