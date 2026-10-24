#include "data/Database.h"
#include "data/SqliteAssignmentRepository.h"
#include "data/SqliteCourseRepository.h"
#include "data/SqliteUserRepository.h"
#include "domain/Errors.h"
#include "domain/UserFactory.h"
#include "services/AdminService.h"
#include "services/Argon2Hasher.h"
#include "services/AssignmentService.h"
#include "services/TenPercentPerDayPolicy.h"

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>

namespace {

// A clock fixed at one time, so the demo gives the same output every run.
class DemoClock : public lms::IClock {
public:
    lms::TimePoint now() const override { return lms::timeFromText("2026-10-24 09:00:00"); }
};

// Runs one action and prints the error message if a rule refuses it.
void attempt(const std::string& description, const std::function<void()>& action) {
    try {
        action();
        std::cout << "  OK:      " << description << "\n";
    } catch (const lms::ValidationError& e) {
        std::cout << "  Refused: " << description << " -> " << e.what() << "\n";
    } catch (const lms::AuthError& e) {
        std::cout << "  Refused: " << description << " -> " << e.what() << "\n";
    }
}

}  // namespace

int main() {
    try {
        // Step 5b demo on an in-memory database, so nothing is saved to lms.db.
        lms::Database db(":memory:");
        db.initialiseSchema();
        lms::UserFactory factory;
        lms::SqliteUserRepository users(db, factory);
        lms::SqliteCourseRepository courses(db);
        lms::SqliteAssignmentRepository assignmentRepo(db);
        lms::Argon2Hasher hasher;
        DemoClock clock;
        lms::AdminService admin(db, users, courses, factory, hasher);
        lms::AssignmentService assignments(assignmentRepo, courses, users, clock);
        lms::TenPercentPerDayPolicy policy;

        // Set-up from step 5a: a department, a course, two lecturers and two students.
        int hod = admin.createUser(lms::Role::Lecturer, "mpho", "Mpho Lecturer", "Password1");
        int science = admin.createDepartment("Science", hod);
        int cs101 = admin.createCourse("CS101", "Programming 1", science);
        int thabo = admin.createUser(lms::Role::Lecturer, "thabo", "Thabo Lecturer", "Password1");
        int lerato = admin.createUser(lms::Role::Lecturer, "lerato", "Lerato Lecturer", "Password1");
        int itu = admin.createUser(lms::Role::Student, "itu", "Itumeleng Student", "Password1");
        int neo = admin.createUser(lms::Role::Student, "neo", "Neo Student", "Password1");
        admin.assignLecturer(cs101, thabo);
        admin.enrol(cs101, itu);
        std::cout << "Today (demo clock): " << lms::toText(clock.now()) << "\n";

        std::cout << "1. Create assignments (FR-07):\n";
        const lms::TimePoint deadline = lms::timeFromText("2026-10-30 17:00:00");
        int a1 = 0;
        attempt("thabo creates Project 1, due 2026-10-30 17:00, out of 100", [&] {
            a1 = assignments.createAssignment(thabo, cs101, "Project 1", deadline, 100);
        });
        attempt("lerato (not CS101's lecturer) creates one", [&] {
            assignments.createAssignment(lerato, cs101, "Quiz", deadline, 20);
        });
        attempt("max mark 0", [&] {
            assignments.createAssignment(thabo, cs101, "Quiz", deadline, 0);
        });
        attempt("deadline yesterday", [&] {
            assignments.createAssignment(thabo, cs101, "Quiz",
                                         lms::timeFromText("2026-10-23 17:00:00"), 20);
        });
        attempt("empty title", [&] {
            assignments.createAssignment(thabo, cs101, "  ", deadline, 20);
        });
        for (const auto& a : assignmentRepo.forCourse(cs101)) {
            std::cout << "           CS101 has: " << a.title << ", due " << lms::toText(a.deadline)
                      << ", max " << a.maxMark << "\n";
        }

        std::cout << "2. Late penalty, raw mark 60 out of 100 (BR-01, BR-02):\n";
        const char* const submittedTimes[] = {
            "2026-10-30 16:59:00", "2026-10-30 17:01:00", "2026-10-31 17:00:00",
            "2026-10-31 17:01:00", "2026-11-02 17:00:00", "2026-11-02 17:01:00"};
        std::cout << std::fixed << std::setprecision(1);
        for (const char* text : submittedTimes) {
            int days = policy.daysLate(lms::timeFromText(text), deadline);
            std::cout << "  submitted " << text << " -> days late " << days << ", final mark "
                      << policy.finalMark(60, 100, days) << "\n";
        }
        std::cout << "  raw 5, 1 day late -> final mark " << policy.finalMark(5, 100, 1)
                  << " (never below 0)\n";

        std::cout << "3. Extensions (FR-08, BR-03):\n";
        attempt("thabo gives itu 3 more days", [&] {
            assignments.grantExtension(thabo, a1, itu, lms::timeFromText("2026-11-02 17:00:00"),
                                       "Medical certificate");
        });
        attempt("new deadline before the normal one", [&] {
            assignments.grantExtension(thabo, a1, itu, lms::timeFromText("2026-10-29 17:00:00"),
                                       "Sick");
        });
        attempt("15 days", [&] {
            assignments.grantExtension(thabo, a1, itu, lms::timeFromText("2026-11-14 17:00:00"),
                                       "Sick");
        });
        attempt("no reason", [&] {
            assignments.grantExtension(thabo, a1, itu, lms::timeFromText("2026-11-01 17:00:00"),
                                       " ");
        });
        attempt("neo, who is not enrolled", [&] {
            assignments.grantExtension(thabo, a1, neo, lms::timeFromText("2026-11-01 17:00:00"),
                                       "Sick");
        });
        attempt("lerato gives itu an extension", [&] {
            assignments.grantExtension(lerato, a1, itu, lms::timeFromText("2026-11-01 17:00:00"),
                                       "Sick");
        });

        const lms::Assignment project = *assignmentRepo.find(a1);
        const auto itusExtension = assignmentRepo.findExtension(a1, itu);
        const lms::TimePoint ituDeadline = project.effectiveDeadline(itusExtension);
        const lms::TimePoint submitted = lms::timeFromText("2026-11-01 12:00:00");
        std::cout << "  itu's deadline is now " << lms::toText(ituDeadline) << " ("
                  << itusExtension->reason << ")\n";
        std::cout << "  Submitted 2026-11-01 12:00: itu is "
                  << policy.daysLate(submitted, ituDeadline) << " days late, without the "
                  << "extension it would be " << policy.daysLate(submitted, project.deadline)
                  << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
