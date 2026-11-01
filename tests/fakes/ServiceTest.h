#pragma once
#include "FakeAssignmentRepository.h"
#include "FakeAuditRepository.h"
#include "FakeClock.h"
#include "FakeCourseRepository.h"
#include "FakePasswordHasher.h"
#include "FakeSubmissionRepository.h"
#include "FakeUserRepository.h"
#include "data/Database.h"
#include "domain/UserFactory.h"
#include "services/AdminService.h"
#include "services/AssignmentService.h"
#include "services/AuditService.h"
#include "services/AuthService.h"
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

// Base class for the service tests. It builds every service the same way System does, but
// with the fake repositories, the fake clock and the fake hasher (dependency injection).
//
// - The Database is an empty in-memory SQLite database. The services only use it to begin
//   and commit transactions; all data lives in the fakes.
// - FileStore is the real class, writing into a fresh temporary folder for each test.
//
// Each test starts at 24 Oct 2026 09:00 with: lecturer thabo teaching CS101, students itu
// and neo enrolled in it, lerato (a lecturer, not of CS101) and sipho (a student, not
// enrolled), and Project 1 due 30 Oct 17:00 out of 100. mpho is the HoD of Science (the
// department of CS101) and admin is the administrator.
class ServiceTest : public ::testing::Test {
protected:
    ServiceTest()
        : folder_(std::filesystem::temp_directory_path() /
                  ("lms_test_" + testName())),
          store(cleanFolder(folder_ / "files")) {
        thabo = addUser(Role::Lecturer, "thabo");
        lerato = addUser(Role::Lecturer, "lerato");
        itu = addUser(Role::Student, "itu");
        neo = addUser(Role::Student, "neo");
        sipho = addUser(Role::Student, "sipho");
        science = courses.saveDepartment(Department{0, "Science"});
        cs101 = courses.saveCourse(Course{0, "CS101", "Programming 1", science});
        courses.addLecturer(cs101, thabo);
        courses.addEnrolment(cs101, itu);
        courses.addEnrolment(cs101, neo);
        mpho = addUser(Role::HoD, "mpho", science);
        admin = addUser(Role::Administrator, "admin");
        project1 = assignmentService.createAssignment(thabo, cs101, "Project 1",
                                                      at("2026-10-30 17:00:00"), 100);
    }

    ~ServiceTest() override {
        std::error_code ignored;
        std::filesystem::remove_all(folder_, ignored);
    }

    int addUser(Role role, const std::string& username,
                std::optional<int> department = std::nullopt) {
        auto user = UserFactory().create(role, username, username + " Test", department);
        user->setPasswordHash(hasher.hash("Password1"));
        return users.save(*user);
    }

    // Writes a small file the "student" can submit, and returns its path.
    std::string makeFile(const std::string& name, const std::string& text = "My work\n") {
        const auto path = folder_ / name;
        std::ofstream(path) << text;
        return path.generic_string();
    }

    int filesStored() const {
        int count = 0;
        for ([[maybe_unused]] const auto& entry :
             std::filesystem::directory_iterator(folder_ / "files")) {
            ++count;
        }
        return count;
    }

private:
    // "AssignmentServiceTest.X" and "SubmissionServiceTest.X" get different folders, so the
    // tests can also run in parallel (ctest -j).
    static std::string testName() {
        const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
        return std::string(info->test_suite_name()) + "_" + info->name();
    }

    static std::string cleanFolder(const std::filesystem::path& folder) {
        std::filesystem::remove_all(folder.parent_path());
        std::filesystem::create_directories(folder);
        return folder.generic_string();
    }

    std::filesystem::path folder_;

protected:
    FakeClock clock;
    FakePasswordHasher hasher;
    TenPercentPerDayPolicy policy;
    Database db{":memory:"};
    FileStore store;
    FakeUserRepository users;
    FakeCourseRepository courses;
    FakeAssignmentRepository assignments;
    FakeSubmissionRepository submissions;
    FakeAuditRepository auditLog{submissions, assignments, courses};
    UserFactory factory;

    AuthService auth{users, hasher, clock};
    AssignmentService assignmentService{db,          assignments, courses, users,
                                        submissions, policy,      clock,   store};
    SubmissionService submissionService{db, submissions, assignments, courses,
                                        policy, clock, store};
    AuditService auditService{auditLog, clock};
    MarkingService markingService{db, submissions, assignments, courses, policy, auditService};
    ModerationService moderationService{db,    submissions,  assignments, courses,
                                        users, auditService, clock};
    AdminService adminService{db, users, courses, factory, hasher};

    int thabo = 0, lerato = 0, itu = 0, neo = 0, sipho = 0, mpho = 0, admin = 0;
    int science = 0, cs101 = 0, project1 = 0;
};

}  // namespace lms::test
