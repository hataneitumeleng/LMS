#include "data/Database.h"
#include "data/DatabaseError.h"
#include "data/SqliteCourseRepository.h"
#include "data/SqliteUserRepository.h"
#include "domain/Errors.h"
#include "domain/UserFactory.h"
#include "services/AdminService.h"
#include "services/Argon2Hasher.h"

#include <functional>
#include <iostream>

namespace {

// Runs one admin action and prints the error message if a rule refuses it.
void attempt(const std::string& description, const std::function<void()>& action) {
    try {
        action();
        std::cout << "  OK:      " << description << "\n";
    } catch (const lms::ValidationError& e) {
        std::cout << "  Refused: " << description << " -> " << e.what() << "\n";
    }
}

}  // namespace

int main() {
    try {
        // Step 5a demo on an in-memory database, so nothing is saved to lms.db.
        lms::Database db(":memory:");
        db.initialiseSchema();
        lms::UserFactory factory;
        lms::SqliteUserRepository users(db, factory);
        lms::SqliteCourseRepository courses(db);
        lms::Argon2Hasher hasher;
        lms::AdminService admin(db, users, courses, factory, hasher);

        std::cout << "1. Create users (FR-03):\n";
        int lect1 = 0, lect2 = 0, stu1 = 0;
        attempt("lecturer mpho", [&] {
            lect1 = admin.createUser(lms::Role::Lecturer, "mpho", "Mpho Lecturer", "Password1");
        });
        attempt("lecturer lerato", [&] {
            lect2 = admin.createUser(lms::Role::Lecturer, "lerato", "Lerato Lecturer", "Password1");
        });
        attempt("student itu", [&] {
            stu1 = admin.createUser(lms::Role::Student, "itu", "Itumeleng Student", "Password1");
        });
        attempt("second user called itu", [&] {
            admin.createUser(lms::Role::Student, "itu", "Someone Else", "Password1");
        });
        attempt("student with a short password", [&] {
            admin.createUser(lms::Role::Student, "neo", "Neo Student", "abc");
        });

        std::cout << "2. Departments and HoDs (FR-04, Section 5.2):\n";
        int science = 0;
        attempt("create Science with mpho as HoD", [&] {
            science = admin.createDepartment("Science", lect1);
        });
        std::cout << "           Science HoD is now: " << users.findActiveHod(science)->username()
                  << " (" << lms::toString(users.findById(lect1)->role()) << ")\n";
        attempt("deactivate mpho while still HoD", [&] { admin.deactivateUser(lect1); });
        attempt("replace Science HoD with lerato", [&] { admin.replaceHod(science, lect2); });
        std::cout << "           Science HoD is now: " << users.findActiveHod(science)->username()
                  << ", mpho active = " << users.findById(lect1)->active() << "\n";

        std::cout << "3. Courses, lecturers and enrolment (FR-04 to FR-06):\n";
        int cs101 = 0;
        attempt("create course CS101", [&] {
            cs101 = admin.createCourse("CS101", "Programming 1", science);
        });
        attempt("create course in department 99", [&] {
            admin.createCourse("XX1", "Nowhere", 99);
        });
        int lect3 = admin.createUser(lms::Role::Lecturer, "thabo", "Thabo Lecturer", "Password1");
        attempt("assign thabo to CS101", [&] { admin.assignLecturer(cs101, lect3); });
        attempt("assign student itu as lecturer", [&] { admin.assignLecturer(cs101, stu1); });
        attempt("enrol itu in CS101", [&] { admin.enrol(cs101, stu1); });
        std::cout << "           itu's courses: ";
        for (const auto& c : courses.coursesForStudent(stu1)) std::cout << c.code << ' ';
        std::cout << "\n";
        attempt("remove itu from CS101", [&] { admin.unenrol(cs101, stu1); });
        std::cout << "           itu's courses now: " << courses.coursesForStudent(stu1).size()
                  << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
