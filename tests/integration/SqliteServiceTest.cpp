// Integration tests: whole use cases through the real services and a real SQLite database,
// including the transaction rules (NFR-07, NFR-08) and backup (FR-20).
// Fixture: see SqliteTest.h.
#include "SqliteTest.h"
#include "app/System.h"

namespace lms::test {
namespace {

class SqliteServiceTest : public SqliteTest {
protected:
    int submitAt(const std::string& time, int student) {
        clock.set(time);
        return submissionService.submit(student, project1, makeFile("work.txt")).id;
    }
    Submission stored(int id) { return *submissions.findById(id); }
};

// ---- The main journey (UC-08 to UC-13) ----

TEST_F(SqliteServiceTest, SubmitMarkModerateRelease) {
    const int ituWork = submitAt("2026-10-29 12:00:00", itu);
    const int neoWork = submitAt("2026-10-31 10:00:00", neo);  // 1 day late

    clock.set("2026-11-02 09:00:00");
    EXPECT_DOUBLE_EQ(markingService.mark(thabo, ituWork, 70, "Good"), 70.0);
    EXPECT_DOUBLE_EQ(markingService.mark(thabo, neoWork, 70, "Good"), 60.0);  // penalty
    EXPECT_EQ(submissionService.listForStudent(itu)[0].status, "Being marked");

    EXPECT_THROW(moderationService.release(mpho, project1), ValidationError);
    EXPECT_EQ(moderationService.approveAll(mpho, project1), 2);
    moderationService.adjust(mpho, neoWork, 65, "Second marker agreed");
    EXPECT_EQ(moderationService.release(mpho, project1), 2);

    EXPECT_EQ(submissionService.listForStudent(itu)[0].status, "Released: 70.0 / 100");
    EXPECT_EQ(submissionService.listForStudent(neo)[0].status, "Released: 65.0 / 100");
    EXPECT_EQ(submissionService.listForStudent(neo)[0].feedback, "Good");
    const auto trail = auditService.entriesFor(*users.findById(mpho));
    ASSERT_EQ(trail.size(), 3u);  // two first marks and the adjustment
    EXPECT_EQ(trail[2].reason, "Second marker agreed");
}

TEST_F(SqliteServiceTest, ExtensionRecalculatesStoredLateness) {  // BR-04
    const int work = submitAt("2026-10-31 09:00:00", itu);  // 16 hours late
    EXPECT_EQ(stored(work).status, SubmissionStatus::Late);
    assignmentService.grantExtension(thabo, project1, itu, at("2026-11-01 17:00:00"), "Ill");
    EXPECT_EQ(stored(work).status, SubmissionStatus::Submitted);
    EXPECT_EQ(stored(work).daysLate, 0);
}

// ---- NFR-08: a mark and its audit entry are saved together or not at all ----

TEST_F(SqliteServiceTest, FailedAuditInsertUndoesTheMark) {
    const int work = submitAt("2026-10-29 12:00:00", itu);
    // From now on the database refuses every audit insert
    sql("CREATE TRIGGER no_audit BEFORE INSERT ON audit_entry "
        "BEGIN SELECT RAISE(ABORT, 'simulated failure'); END");
    EXPECT_THROW(markingService.mark(thabo, work, 70, "Good"), DatabaseError);

    const Submission s = stored(work);
    EXPECT_EQ(s.status, SubmissionStatus::Submitted);  // the mark was rolled back
    EXPECT_FALSE(s.rawMark.has_value());
    EXPECT_TRUE(auditLog.findAll().empty());

    sql("DROP TRIGGER no_audit");  // the problem is fixed: marking works again
    EXPECT_DOUBLE_EQ(markingService.mark(thabo, work, 70, "Good"), 70.0);
    EXPECT_EQ(auditLog.findAll().size(), 1u);
}

TEST_F(SqliteServiceTest, FailedAuditInsertUndoesTheAdjustment) {
    const int work = submitAt("2026-10-29 12:00:00", itu);
    markingService.mark(thabo, work, 70, "Good");
    sql("CREATE TRIGGER no_audit BEFORE INSERT ON audit_entry "
        "BEGIN SELECT RAISE(ABORT, 'simulated failure'); END");
    EXPECT_THROW(moderationService.adjust(mpho, work, 50, "Too generous"), DatabaseError);
    EXPECT_EQ(stored(work).status, SubmissionStatus::Marked);
    EXPECT_DOUBLE_EQ(*stored(work).finalMark, 70.0);
    EXPECT_EQ(auditLog.findAll().size(), 1u);  // only the first mark
}

// ---- NFR-07, Figure 6: a failed submission leaves no row and no file ----

TEST_F(SqliteServiceTest, FailedVersionInsertLeavesNothingBehind) {
    sql("CREATE TRIGGER no_version BEFORE INSERT ON submission_version "
        "BEGIN SELECT RAISE(ABORT, 'simulated failure'); END");
    clock.set("2026-10-29 12:00:00");
    EXPECT_THROW(submissionService.submit(itu, project1, makeFile("work.txt")), DatabaseError);
    EXPECT_FALSE(submissions.findCurrent(itu, project1));  // the submission row was undone
    EXPECT_EQ(filesStored(), 0);                           // and the copy deleted
}

TEST_F(SqliteServiceTest, ReleaseIsAllOrNothing) {  // FR-15, NFR-08
    const int a = submitAt("2026-10-29 12:00:00", itu);
    const int b = submitAt("2026-10-29 12:00:00", neo);
    markingService.mark(thabo, a, 70, "Good");
    markingService.mark(thabo, b, 60, "Fair");
    moderationService.approveAll(mpho, project1);
    // The second release update fails, so the first must be undone too
    sql("CREATE TRIGGER one_release AFTER UPDATE OF status ON submission "
        "WHEN NEW.status = 'Released' AND "
        "(SELECT COUNT(*) FROM submission WHERE status = 'Released') > 1 "
        "BEGIN SELECT RAISE(ABORT, 'simulated failure'); END");
    EXPECT_THROW(moderationService.release(mpho, project1), DatabaseError);
    EXPECT_EQ(stored(a).status, SubmissionStatus::Moderated);
    EXPECT_EQ(stored(b).status, SubmissionStatus::Moderated);
}

TEST_F(SqliteServiceTest, ReplaceHodIsAllOrNothing) {  // Section 5.2
    sql("CREATE TRIGGER no_new_hod BEFORE UPDATE OF role ON user "
        "WHEN NEW.role = 'HoD' BEGIN SELECT RAISE(ABORT, 'simulated failure'); END");
    EXPECT_THROW(adminService.replaceHod(science, lerato), DatabaseError);
    EXPECT_TRUE(users.findById(mpho)->active());  // the old HoD was not deactivated
    EXPECT_EQ(users.findActiveHod(science)->id(), mpho);
}

// ---- Login against the stored data ----

TEST_F(SqliteServiceTest, LockSurvivesARestart) {  // NFR-04
    for (int i = 0; i < 5; ++i) EXPECT_THROW(auth.login("itu", "wrong"), AuthError);
    // A "restarted program": new connection, new repositories, new AuthService
    Database reopened(dbPath);
    SqliteUserRepository otherUsers(reopened, factory);
    AuthService otherAuth(otherUsers, hasher, clock);
    EXPECT_THROW(otherAuth.login("itu", "Password1"), AuthError);
    clock.set("2026-10-24 09:05:00");
    EXPECT_NO_THROW(otherAuth.login("itu", "Password1"));
}

// ---- Backup (FR-20, NFR-11) and the start-up clean-up ----

TEST_F(SqliteServiceTest, BackupCopiesTheDatabaseAndTheFiles) {
    submitAt("2026-10-29 12:00:00", itu);
    const std::string target = folder() + "/backup1";
    backupService.backup(target);

    EXPECT_TRUE(std::filesystem::exists(target + "/lms.db"));
    EXPECT_EQ(std::distance(std::filesystem::directory_iterator(target + "/submissions"),
                            std::filesystem::directory_iterator{}),
              1);
    Database copy(target + "/lms.db");  // the backup is a working database
    SqliteSubmissionRepository copied(copy);
    EXPECT_TRUE(copied.findCurrent(itu, project1));
}

TEST_F(SqliteServiceTest, BackupRefusesAnExistingFolder) {
    const std::string target = folder() + "/backup1";
    backupService.backup(target);
    EXPECT_THROW(backupService.backup(target), ValidationError);
}

TEST_F(SqliteServiceTest, OrphanFilesAreRemovedAtStartUp) {
    submitAt("2026-10-29 12:00:00", itu);
    assignmentService.attachBrief(thabo, project1, makeFile("brief.pdf", "Do this"));
    std::ofstream(filesFolder + "/sub99_v1.txt") << "left by a crash\n";
    EXPECT_EQ(submissionService.removeOrphanFiles(), 1);
    EXPECT_EQ(filesStored(), 2);
}

// ---- The composition root both programs start from ----

TEST(SystemTest, BuildsEverythingOnANewDatabase) {
    const auto folder = std::filesystem::temp_directory_path() / "lms_it_SystemTest";
    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder);
    const std::string dbFile = (folder / "lms.db").generic_string();
    const std::string files = (folder / "submissions").generic_string();
    {
        System system(dbFile, files);  // creates the tables, like the first start
        system.services().admin.createUser(Role::Administrator, "admin", "System Admin",
                                           "Password1");
        // the real Argon2 hasher and the real clock are used here
        EXPECT_EQ(system.services().auth.login("admin", "Password1")->role(),
                  Role::Administrator);
    }
    {
        System restarted(dbFile, files);  // the next start finds the same data
        EXPECT_EQ(restarted.services().admin.listUsers().size(), 1u);
        EXPECT_THROW(restarted.services().auth.login("admin", "Wrong1234"), AuthError);
    }
    std::filesystem::remove_all(folder);
}

}  // namespace
}  // namespace lms::test
