// Integration tests: the SQLite repositories and the database schema, on a real database
// file. They check what the fakes cannot: the SQL, the joins, the constraints in
// sql/schema.sql and that data is really stored. Fixture: see SqliteTest.h.
#include "SqliteTest.h"

namespace lms::test {
namespace {

using SqliteRepositoryTest = SqliteTest;

// ---- Users (FR-02, FR-03, NFR-04) ----

TEST_F(SqliteRepositoryTest, UserIsReadBackAsTheRightSubclass) {
    const auto hod = users.findById(mpho);
    ASSERT_TRUE(hod);
    EXPECT_EQ(hod->role(), Role::HoD);
    EXPECT_EQ(hod->departmentId(), science);
    EXPECT_EQ(hod->username(), "mpho");
    EXPECT_EQ(users.findByUsername("itu")->role(), Role::Student);
    EXPECT_EQ(users.findActiveHod(science)->id(), mpho);
    EXPECT_EQ(users.findAll().size(), 6u);
}

TEST_F(SqliteRepositoryTest, LockoutIsStoredInTheDatabase) {  // NFR-04
    for (int i = 0; i < 5; ++i) EXPECT_THROW(auth.login("itu", "wrong"), AuthError);
    const auto saved = users.findByUsername("itu");
    ASSERT_TRUE(saved->lockedUntil());
    EXPECT_EQ(*saved->lockedUntil(), at("2026-10-24 09:05:00"));
}

TEST_F(SqliteRepositoryTest, DatabaseRefusesADuplicateUsername) {
    auto copy = factory.create(Role::Student, "itu", "Second Itu", std::nullopt);
    copy->setPasswordHash("x");
    EXPECT_THROW(users.save(*copy), DatabaseError);  // UNIQUE(username)
}

TEST_F(SqliteRepositoryTest, DatabaseAllowsOnlyOneActiveHodPerDepartment) {
    auto second = factory.create(Role::HoD, "boss2", "Second HoD", science);
    second->setPasswordHash("x");
    EXPECT_THROW(users.save(*second), DatabaseError);  // index one_active_hod_per_dept
}

// ---- Courses, teaching and enrolment (FR-05, FR-06) ----

TEST_F(SqliteRepositoryTest, TeachingAndEnrolmentAreStored) {
    EXPECT_TRUE(courses.isLecturerOf(cs101, thabo));
    EXPECT_FALSE(courses.isLecturerOf(cs101, lerato));
    EXPECT_TRUE(courses.isEnrolled(cs101, itu));
    EXPECT_FALSE(courses.isEnrolled(cs101, sipho));
    ASSERT_EQ(courses.coursesForStudent(itu).size(), 1u);
    EXPECT_EQ(courses.coursesForStudent(itu)[0].code, "CS101");
    EXPECT_EQ(courses.coursesForLecturer(thabo).size(), 1u);
    courses.removeEnrolment(cs101, itu);
    EXPECT_FALSE(courses.isEnrolled(cs101, itu));
}

TEST_F(SqliteRepositoryTest, DuplicateCourseCodeIsRefused) {
    EXPECT_THROW(adminService.createCourse("CS101", "Copy", science), DatabaseError);
}

// ---- Assignments, extensions and briefs (FR-07, FR-08, FR-24) ----

TEST_F(SqliteRepositoryTest, AssignmentIsStoredWithItsDeadline) {
    const auto a = assignments.find(project1);
    ASSERT_TRUE(a);
    EXPECT_EQ(a->title, "Project 1");
    EXPECT_EQ(a->deadline, at("2026-10-30 17:00:00"));
    EXPECT_EQ(a->maxMark, 100);
    EXPECT_EQ(a->createdBy, thabo);
    EXPECT_TRUE(a->briefPath.empty());
    EXPECT_EQ(assignments.forCourse(cs101).size(), 1u);
}

TEST_F(SqliteRepositoryTest, BriefPathIsStored) {
    assignmentService.attachBrief(thabo, project1, makeFile("brief.pdf", "Do this"));
    const auto a = assignments.find(project1);
    EXPECT_FALSE(a->briefPath.empty());
    ASSERT_EQ(assignments.allBriefPaths().size(), 1u);
    EXPECT_EQ(assignments.allBriefPaths()[0], a->briefPath);
}

TEST_F(SqliteRepositoryTest, SecondExtensionReplacesTheFirstRow) {
    assignmentService.grantExtension(thabo, project1, itu, at("2026-11-02 17:00:00"), "Ill");
    assignmentService.grantExtension(thabo, project1, itu, at("2026-11-05 17:00:00"), "Still ill");
    const auto ext = assignments.findExtension(project1, itu);
    ASSERT_TRUE(ext);
    EXPECT_EQ(ext->newDeadline, at("2026-11-05 17:00:00"));
    EXPECT_EQ(ext->reason, "Still ill");
    EXPECT_FALSE(assignments.findExtension(project1, neo));
}

// ---- Submissions and versions (FR-10, FR-11, BR-05) ----

TEST_F(SqliteRepositoryTest, VersionsAreKeptInOrder) {
    clock.set("2026-10-28 09:00:00");
    const int id = submissionService.submit(itu, project1, makeFile("v1.txt")).id;
    clock.set("2026-10-29 09:00:00");
    submissionService.submit(itu, project1, makeFile("v2.txt"));
    const auto versions = submissions.versions(id);
    ASSERT_EQ(versions.size(), 2u);
    EXPECT_EQ(versions[0].versionNo, 1);
    EXPECT_EQ(versions[1].versionNo, 2);
    EXPECT_EQ(versions[1].submittedAt, at("2026-10-29 09:00:00"));
    EXPECT_EQ(submissions.latestVersion(id)->versionNo, 2);
    EXPECT_EQ(submissions.allFilePaths().size(), 2u);
}

TEST_F(SqliteRepositoryTest, CountNotModeratedFollowsTheStatus) {  // FR-15
    clock.set("2026-10-28 09:00:00");
    const int a = submissionService.submit(itu, project1, makeFile("a.txt")).id;
    submissionService.submit(neo, project1, makeFile("b.txt"));
    EXPECT_EQ(submissions.countNotModerated(project1), 2);
    markingService.mark(thabo, a, 70, "Good");
    EXPECT_EQ(submissions.countNotModerated(project1), 2);  // Marked still counts
    moderationService.approveAll(mpho, project1);
    EXPECT_EQ(submissions.countNotModerated(project1), 1);  // neo is not marked
}

TEST_F(SqliteRepositoryTest, SchemaRefusesAMarkWithoutFeedback) {  // defence in depth
    clock.set("2026-10-28 09:00:00");
    Submission s = submissionService.submit(itu, project1, makeFile("a.txt"));
    s.recordMark(70, 70, "", thabo);  // skips MarkingService's own check
    EXPECT_THROW(submissions.save(s), DatabaseError);
}

TEST_F(SqliteRepositoryTest, OneSubmissionPerStudentPerAssignment) {  // BR-05
    clock.set("2026-10-28 09:00:00");
    submissionService.submit(itu, project1, makeFile("a.txt"));
    Submission copy;
    copy.assignmentId = project1;
    copy.studentId = itu;
    EXPECT_THROW(submissions.save(copy), DatabaseError);
}

// ---- Audit trail (FR-17) ----

TEST_F(SqliteRepositoryTest, AuditJoinFindsTheDepartment) {
    clock.set("2026-10-28 09:00:00");
    const int work = submissionService.submit(itu, project1, makeFile("a.txt")).id;
    markingService.mark(thabo, work, 70, "Good");
    const int arts = adminService.createDepartment("Arts", lerato);

    ASSERT_EQ(auditLog.findByDepartment(science).size(), 1u);
    EXPECT_TRUE(auditLog.findByDepartment(arts).empty());
    const AuditEntry e = auditLog.findAll().at(0);
    EXPECT_EQ(e.submissionId, work);
    EXPECT_EQ(e.userId, thabo);
    EXPECT_FALSE(e.oldValue.has_value());  // NULL in the database
    EXPECT_DOUBLE_EQ(e.newValue, 70.0);
}

// ---- The database file itself ----

TEST_F(SqliteRepositoryTest, DataIsStillThereAfterReopening) {
    Database reopened(dbPath);
    reopened.initialiseSchema();  // must not create the tables again
    SqliteUserRepository otherUsers(reopened, factory);
    SqliteAssignmentRepository otherAssignments(reopened);
    EXPECT_EQ(otherUsers.findAll().size(), 6u);
    EXPECT_EQ(otherAssignments.find(project1)->title, "Project 1");
}

TEST_F(SqliteRepositoryTest, OldDatabaseGetsTheBriefColumn) {  // FR-24 upgrade
    sql("ALTER TABLE assignment DROP COLUMN brief_path");  // as a Phase 4 database was
    Database reopened(dbPath);
    reopened.initialiseSchema();
    SqliteAssignmentRepository upgraded(reopened);
    const auto a = upgraded.find(project1);
    ASSERT_TRUE(a);  // the old row is still there
    EXPECT_TRUE(a->briefPath.empty());
}

}  // namespace
}  // namespace lms::test
