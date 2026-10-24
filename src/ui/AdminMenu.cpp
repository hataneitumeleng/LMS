#include "ui/AdminMenu.h"

namespace lms {

// ---- Main administrator menu ----

AdminMenu::AdminMenu(Console& io, Services& services, const User& user)
    : Menu(io, services), user_(user) {}

std::string AdminMenu::title() const { return "Administrator menu: " + user_.fullName(); }

std::vector<std::string> AdminMenu::options() const {
    return {"Users", "Departments, courses and enrolment", "View the audit trail",
            "Back up the system", "Change my password"};
}

void AdminMenu::handle(int choice) {
    switch (choice) {
        case 1: AdminUsersMenu(io_, services_).run(); break;
        case 2: AdminCoursesMenu(io_, services_).run(); break;
        case 3: showAudit(services_.audit.entriesFor(user_)); break;
        case 4: backUp(); break;
        case 5: changePassword(user_); break;
    }
}

void AdminMenu::backUp() {
    const std::string folder = io_.text("New folder for the backup, e.g. backups/2026-10-24");
    services_.backup.backup(folder);
    io_.print("Backup saved in " + folder + " (lms.db and the submitted files).");
}

// ---- Users sub-menu ----

AdminUsersMenu::AdminUsersMenu(Console& io, Services& services) : Menu(io, services) {}

std::vector<std::string> AdminUsersMenu::options() const {
    return {"List users", "Create a user", "Change a user's name", "Deactivate a user"};
}

void AdminUsersMenu::handle(int choice) {
    switch (choice) {
        case 1: listUsers(); break;
        case 2: createUser(); break;
        case 3: renameUser(); break;
        case 4: deactivateUser(); break;
    }
}

void AdminUsersMenu::listUsers() {
    for (const auto& user : services_.admin.listUsers()) {
        std::string line = "[" + std::to_string(user->id()) + "] " + user->username() + "  " +
                           user->fullName() + "  " + toString(user->role());
        if (user->departmentId()) line += " of department " + std::to_string(*user->departmentId());
        if (!user->active()) line += "  (deactivated)";
        io_.print(line);
    }
}

void AdminUsersMenu::createUser() {
    io_.print("1. Student  2. Lecturer  3. Administrator");
    io_.print("(An HoD is made by choosing a lecturer when creating a department.)");
    const int roleChoice = io_.choice("Role", 1, 3);
    const Role role = roleChoice == 1   ? Role::Student
                      : roleChoice == 2 ? Role::Lecturer
                                        : Role::Administrator;
    const std::string username = io_.text("Username");
    const std::string fullName = io_.text("Full name");
    const std::string password = io_.text("First password (at least 8 characters)");
    const int id = services_.admin.createUser(role, username, fullName, password);
    io_.print("User created with number " + std::to_string(id) + ".");
}

void AdminUsersMenu::renameUser() {
    const int id = io_.number("User number");
    services_.admin.updateUserName(id, io_.text("New full name"));
    io_.print("Name changed.");
}

void AdminUsersMenu::deactivateUser() {
    const int id = io_.number("User number");
    // NFR-06: confirm before an action that locks someone out
    if (!io_.confirm("Deactivate this user? They will not be able to log in")) return;
    services_.admin.deactivateUser(id);
    io_.print("User deactivated.");
}

// ---- Departments, courses and enrolment sub-menu ----

AdminCoursesMenu::AdminCoursesMenu(Console& io, Services& services) : Menu(io, services) {}

std::vector<std::string> AdminCoursesMenu::options() const {
    return {"List departments and courses", "Create a department and its HoD",
            "Rename a department",           "Replace a department's HoD",
            "Create a course",               "Change a course's code or title",
            "Assign a lecturer to a course", "Enrol a student",
            "Remove a student from a course"};
}

void AdminCoursesMenu::handle(int choice) {
    switch (choice) {
        case 1: listAll(); break;
        case 2: createDepartment(); break;
        case 3: renameDepartment(); break;
        case 4: replaceHod(); break;
        case 5: createCourse(); break;
        case 6: updateCourse(); break;
        case 7: assignLecturer(); break;
        case 8: enrol(); break;
        case 9: unenrol(); break;
    }
}

void AdminCoursesMenu::listAll() {
    const auto users = services_.admin.listUsers();
    for (const Department& d : services_.admin.listDepartments()) {
        std::string hod = "none";
        for (const auto& user : users) {
            if (user->role() == Role::HoD && user->active() && user->departmentId() == d.id) {
                hod = user->fullName();
            }
        }
        io_.print("Department [" + std::to_string(d.id) + "] " + d.name + "  HoD: " + hod);
        for (const Course& c : services_.admin.listCourses()) {
            if (c.departmentId == d.id) {
                io_.print("      Course [" + std::to_string(c.id) + "] " + c.code + " " + c.title);
            }
        }
    }
}

void AdminCoursesMenu::createDepartment() {
    const std::string name = io_.text("Department name");
    const int lecturerId = io_.number("Number of the lecturer who will be HoD");
    const int id = services_.admin.createDepartment(name, lecturerId);
    io_.print("Department created with number " + std::to_string(id) + ".");
}

void AdminCoursesMenu::renameDepartment() {
    const int id = io_.number("Department number");
    services_.admin.renameDepartment(id, io_.text("New name"));
    io_.print("Department renamed.");
}

void AdminCoursesMenu::replaceHod() {
    const int departmentId = io_.number("Department number");
    const int lecturerId = io_.number("Number of the lecturer who will be the new HoD");
    if (!io_.confirm("The current HoD will be deactivated. Continue?")) return;
    services_.admin.replaceHod(departmentId, lecturerId);
    io_.print("HoD replaced.");
}

void AdminCoursesMenu::createCourse() {
    const std::string code = io_.text("Course code");
    const std::string title = io_.text("Course title");
    const int departmentId = io_.number("Department number");
    const int id = services_.admin.createCourse(code, title, departmentId);
    io_.print("Course created with number " + std::to_string(id) + ".");
}

void AdminCoursesMenu::updateCourse() {
    const int id = io_.number("Course number");
    const std::string code = io_.text("New code");
    services_.admin.updateCourse(id, code, io_.text("New title"));
    io_.print("Course changed.");
}

void AdminCoursesMenu::assignLecturer() {
    const int courseId = io_.number("Course number");
    services_.admin.assignLecturer(courseId, io_.number("Lecturer number"));
    io_.print("Lecturer assigned.");
}

void AdminCoursesMenu::enrol() {
    const int courseId = io_.number("Course number");
    services_.admin.enrol(courseId, io_.number("Student number"));
    io_.print("Student enrolled.");
}

void AdminCoursesMenu::unenrol() {
    const int courseId = io_.number("Course number");
    const int studentId = io_.number("Student number");
    if (!io_.confirm("Remove this student from the course?")) return;
    services_.admin.unenrol(courseId, studentId);
    io_.print("Student removed from the course.");
}

}  // namespace lms
