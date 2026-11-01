#pragma once
#include "../fakes/FakeClock.h"
#include "../fakes/FakePasswordHasher.h"
#include "data/Database.h"
#include "data/DatabaseError.h"
#include "data/SqliteAssignmentRepository.h"
#include "data/SqliteAuditRepository.h"
#include "data/SqliteCourseRepository.h"
#include "data/SqliteSubmissionRepository.h"
#include "data/SqliteUserRepository.h"
#include "domain/Errors.h"
#include "domain/UserFactory.h"
#include "services/AdminService.h"
#include "services/AssignmentService.h"
#include "services/AuditService.h"
#include "services/AuthService.h"
#include "services/BackupService.h"
#include "services/FileStore.h"
#include "services/MarkingService.h"
#include "services/ModerationService.h"
#include "services/SubmissionService.h"
#include "services/TenPercentPerDayPolicy.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace lms::test {

// Base class for the integration tests: the real SQLite repositories on a real database
// file, wired exactly as System does. Only the clock and the password hasher are fakes
// (so time can be set, and Argon2 does not slow every test down).
//
// Each test gets a new folder with a new lms.db, and the administrator sets up the same
// world as the service tests through AdminService, so that path is used for real too:
// thabo teaches CS101 in Science (HoD mpho), itu and neo are enrolled, lerato is a second
// lecturer, sipho is not enrolled, and Project 1 is due 30 Oct 17:00 out of 100.
class SqliteTest : public ::testing::Test {
protected:
    SqliteTest()
        : folder_(std::filesystem::temp_directory_path() / ("lms_it_" + testName())),
          dbPath(prepare(folder_) + "/lms.db"),
          filesFolder((folder_ / "files").generic_string()),
          db(dbPath),
          store(filesFolder) {
        db.initialiseSchema();
        clock.set("2026-10-24 09:00:00");
        thabo = adminService.createUser(Role::Lecturer, "thabo", "Thabo T", "Password1");
        lerato = adminService.createUser(Role::Lecturer, "lerato", "Lerato L", "Password1");
        mpho = adminService.createUser(Role::Lecturer, "mpho", "Mpho M", "Password1");
        itu = adminService.createUser(Role::Student, "itu", "Itu I", "Password1");
        neo = adminService.createUser(Role::Student, "neo", "Neo N", "Password1");
        sipho = adminService.createUser(Role::Student, "sipho", "Sipho S", "Password1");
        science = adminService.createDepartment("Science", mpho);
        cs101 = adminService.createCourse("CS101", "Programming 1", science);
        adminService.assignLecturer(cs101, thabo);
        adminService.enrol(cs101, itu);
        adminService.enrol(cs101, neo);
        project1 = assignmentService.createAssignment(thabo, cs101, "Project 1",
                                                      at("2026-10-30 17:00:00"), 100);
    }

    ~SqliteTest() override {
        std::error_code ignored;
        std::filesystem::remove_all(folder_, ignored);
    }

    // Runs one SQL statement directly, e.g. to add a trigger that makes an insert fail.
    void sql(const std::string& text) {
        Statement stmt = db.prepare(text);
        stmt.step();
    }

    std::string makeFile(const std::string& name, const std::string& text = "My work\n") {
        const auto path = folder_ / name;
        std::ofstream(path) << text;
        return path.generic_string();
    }

    int filesStored() const {
        int count = 0;
        if (!std::filesystem::exists(filesFolder)) return 0;
        for ([[maybe_unused]] const auto& entry :
             std::filesystem::directory_iterator(filesFolder)) {
            ++count;
        }
        return count;
    }

    std::string folder() const { return folder_.generic_string(); }

private:
    static std::string testName() {
        const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
        return std::string(info->test_suite_name()) + "_" + info->name();
    }
    static std::string prepare(const std::filesystem::path& folder) {
        std::filesystem::remove_all(folder);
        std::filesystem::create_directories(folder / "files");
        return folder.generic_string();
    }

    std::filesystem::path folder_;

protected:
    std::string dbPath;
    std::string filesFolder;
    FakeClock clock;
    FakePasswordHasher hasher;
    TenPercentPerDayPolicy policy;
    UserFactory factory;
    Database db;
    FileStore store;

    SqliteUserRepository users{db, factory};
    SqliteCourseRepository courses{db};
    SqliteAssignmentRepository assignments{db};
    SqliteSubmissionRepository submissions{db};
    SqliteAuditRepository auditLog{db};

    AuthService auth{users, hasher, clock};
    AdminService adminService{db, users, courses, factory, hasher};
    AssignmentService assignmentService{db,          assignments, courses, users,
                                        submissions, policy,      clock,   store};
    SubmissionService submissionService{db, submissions, assignments, courses,
                                        policy, clock, store};
    AuditService auditService{auditLog, clock};
    MarkingService markingService{db, submissions, assignments, courses, policy, auditService};
    ModerationService moderationService{db,    submissions,  assignments, courses,
                                        users, auditService, clock};
    BackupService backupService{db, filesFolder};

    int thabo = 0, lerato = 0, mpho = 0, itu = 0, neo = 0, sipho = 0;
    int science = 0, cs101 = 0, project1 = 0;
};

}  // namespace lms::test
