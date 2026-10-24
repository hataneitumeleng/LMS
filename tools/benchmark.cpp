// R12 / NFR-01 / NFR-02 benchmark.
// Fills a fresh database with the size from the requirements (800 students,
// 20 courses) and times login and the list screens through the real services.
// Run from the project folder:  .\build\lms_benchmark.exe
#include "data/Database.h"
#include "data/SqliteAssignmentRepository.h"
#include "data/SqliteAuditRepository.h"
#include "data/SqliteCourseRepository.h"
#include "data/SqliteSubmissionRepository.h"
#include "data/SqliteUserRepository.h"
#include "data/Transaction.h"
#include "domain/UserFactory.h"
#include "services/AdminService.h"
#include "services/Argon2Hasher.h"
#include "services/AssignmentService.h"
#include "services/AuditService.h"
#include "services/AuthService.h"
#include "services/MarkingService.h"
#include "services/ModerationService.h"
#include "services/SubmissionService.h"
#include "services/TenPercentPerDayPolicy.h"

#include <chrono>
#include <cstdio>
#include <functional>
#include <iomanip>
#include <iostream>

namespace {

constexpr int kStudents = 800;          // NFR-01
constexpr int kCourses = 20;            // NFR-01
constexpr int kCoursesPerStudent = 5;   // so each course has 200 students
constexpr int kAssignmentsPerCourse = 5;

bool allPassed = true;

// Runs the action once and prints its time against the limit.
void timeIt(const std::string& name, double limitSeconds, const std::function<void()>& action) {
    const auto start = std::chrono::steady_clock::now();
    action();
    const std::chrono::duration<double> took = std::chrono::steady_clock::now() - start;
    const bool pass = took.count() <= limitSeconds;
    if (!pass) allPassed = false;
    std::cout << "  " << std::left << std::setw(52) << name << std::right << std::fixed
              << std::setprecision(3) << took.count() << " s  (limit " << std::setprecision(0)
              << limitSeconds << " s)  " << (pass ? "PASS" : "FAIL") << "\n";
}

}  // namespace

int main() {
    try {
        const char* const path = "benchmark.db";
        std::remove(path);
        lms::Database db(path);
        db.initialiseSchema();

        lms::SystemClock clock;
        lms::UserFactory factory;
        lms::Argon2Hasher hasher;
        lms::TenPercentPerDayPolicy policy;
        lms::FileStore store("build/benchmark_files");
        lms::SqliteUserRepository users(db, factory);
        lms::SqliteCourseRepository courses(db);
        lms::SqliteAssignmentRepository assignmentRepo(db);
        lms::SqliteSubmissionRepository submissionRepo(db);
        lms::SqliteAuditRepository auditRepo(db);
        lms::AuthService auth(users, hasher, clock);
        lms::AdminService admin(db, users, courses, factory, hasher);
        lms::SubmissionService submissions(db, submissionRepo, assignmentRepo, courses, policy,
                                           clock, store);
        lms::AuditService audit(auditRepo, clock);
        lms::MarkingService marking(db, submissionRepo, assignmentRepo, courses, policy, audit);
        lms::ModerationService moderation(db, submissionRepo, assignmentRepo, courses, users, audit,
                                          clock);

        // ---- Fill the database ----
        std::cout << "Filling benchmark.db: " << kStudents << " students, " << kCourses
                  << " courses, " << kCourses * kAssignmentsPerCourse << " assignments...\n";
        // Hashing is slow on purpose (NFR-03), so one hash is shared by all test users.
        // Each real user gets their own salt; this shortcut is only for the test data.
        const std::string sharedHash = hasher.hash("Password1");
        int adminId = 0, hodId = 0, lecturerId = 0, firstStudent = 0;
        std::vector<int> assignmentIds;
        {
            lms::Transaction tx(db);
            auto addUser = [&](lms::Role role, const std::string& name) {
                auto user = factory.create(role, name, name, std::nullopt);
                user->setPasswordHash(sharedHash);
                return users.save(*user);
            };
            adminId = addUser(lms::Role::Administrator, "admin");
            hodId = addUser(lms::Role::Lecturer, "hod");
            lecturerId = addUser(lms::Role::Lecturer, "lecturer");
            const int department = courses.saveDepartment(lms::Department{0, "Science"});
            auto hod = factory.create(lms::Role::HoD, "hod", "hod", department);
            hod->setId(hodId);
            hod->setPasswordHash(sharedHash);
            users.save(*hod);

            std::vector<int> courseIds;
            for (int c = 1; c <= kCourses; ++c) {
                const int id = courses.saveCourse(
                    lms::Course{0, "C" + std::to_string(c), "Course " + std::to_string(c), department});
                courses.addLecturer(id, lecturerId);
                courseIds.push_back(id);
                for (int a = 1; a <= kAssignmentsPerCourse; ++a) {
                    lms::Assignment assignment;
                    assignment.courseId = id;
                    assignment.title = "Assignment " + std::to_string(a);
                    assignment.deadline = clock.now() - std::chrono::hours(24 * a);
                    assignment.maxMark = 100;
                    assignment.createdBy = lecturerId;
                    assignmentIds.push_back(assignmentRepo.save(assignment));
                }
            }
            for (int s = 0; s < kStudents; ++s) {
                const int studentId = addUser(lms::Role::Student, "student" + std::to_string(s));
                if (s == 0) firstStudent = studentId;
                for (int k = 0; k < kCoursesPerStudent; ++k) {
                    const int courseIndex = (s + k * 4) % kCourses;
                    courses.addEnrolment(courseIds[courseIndex], studentId);
                    for (int a = 0; a < kAssignmentsPerCourse; ++a) {
                        const int assignmentId =
                            assignmentIds[courseIndex * kAssignmentsPerCourse + a];
                        // Every student has submitted every assignment, and it is marked.
                        lms::Submission sub;
                        sub.assignmentId = assignmentId;
                        sub.studentId = studentId;
                        sub.recordMark(60, 60, "Good", lecturerId);
                        sub.id = submissionRepo.save(sub);
                        submissionRepo.addVersion(
                            sub.id, lms::SubmissionVersion{1, "none", clock.now()});
                        audit.record(lecturerId, sub.id, std::nullopt, 60, "First mark");
                    }
                }
            }
            tx.commit();
        }
        std::cout << "  " << kStudents * kCoursesPerStudent * kAssignmentsPerCourse
                  << " submissions and audit entries created\n\n";

        // ---- Time the screens through the real services ----
        std::cout << "Timings (each screen runs once, as a user would):\n";
        timeIt("NFR-01 login (student, Argon2id check)", 2,
               [&] { auth.login("student0", "Password1"); });
        timeIt("NFR-02 student: my assignments (25 rows)", 3,
               [&] { submissions.listForStudent(firstStudent); });
        timeIt("NFR-02 lecturer: submissions for one assignment", 3,
               [&] { marking.listForMarking(lecturerId, assignmentIds[0]); });
        timeIt("NFR-02 HoD: assignments in department (100 rows)", 3,
               [&] { moderation.assignmentsInDepartment(hodId); });
        timeIt("NFR-02 HoD: marked work for one assignment", 3,
               [&] { moderation.listForModeration(hodId, assignmentIds[0]); });
        timeIt("NFR-02 admin: list all users", 3, [&] { admin.listUsers(); });
        timeIt("NFR-02 admin: full audit trail (20,000 rows)", 3,
               [&] { audit.entriesFor(*users.findById(adminId)); });
        timeIt("FR-14 HoD: approve all (200 submissions)", 3,
               [&] { moderation.approveAll(hodId, assignmentIds[0]); });
        timeIt("FR-15 HoD: release (200 submissions)", 3,
               [&] { moderation.release(hodId, assignmentIds[0]); });

        std::cout << "\n" << (allPassed ? "All timings are within the limits. R12 can be closed."
                                        : "Some timings are over the limit.")
                  << "\n";
        return allPassed ? 0 : 1;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
        return 1;
    }
}
