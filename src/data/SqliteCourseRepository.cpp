#include "data/SqliteCourseRepository.h"

namespace lms {

SqliteCourseRepository::SqliteCourseRepository(Database& db) : db_(db) {}

int SqliteCourseRepository::saveDepartment(const Department& department) {
    if (department.id == 0) {
        Statement stmt = db_.prepare("INSERT INTO department (name) VALUES (?)");
        stmt.bind(1, department.name);
        stmt.step();
        return static_cast<int>(db_.lastInsertId());
    }
    Statement stmt = db_.prepare("UPDATE department SET name = ? WHERE id = ?");
    stmt.bind(1, department.name).bind(2, department.id);
    stmt.step();
    return department.id;
}

int SqliteCourseRepository::saveCourse(const Course& course) {
    if (course.id == 0) {
        Statement stmt =
            db_.prepare("INSERT INTO course (code, title, department_id) VALUES (?, ?, ?)");
        stmt.bind(1, course.code).bind(2, course.title).bind(3, course.departmentId);
        stmt.step();
        return static_cast<int>(db_.lastInsertId());
    }
    Statement stmt =
        db_.prepare("UPDATE course SET code = ?, title = ?, department_id = ? WHERE id = ?");
    stmt.bind(1, course.code).bind(2, course.title).bind(3, course.departmentId).bind(4, course.id);
    stmt.step();
    return course.id;
}

std::optional<Department> SqliteCourseRepository::findDepartment(int id) {
    Statement stmt = db_.prepare("SELECT id, name FROM department WHERE id = ?");
    stmt.bind(1, id);
    if (!stmt.step()) return std::nullopt;
    return Department{stmt.columnInt(0), stmt.columnText(1)};
}

std::optional<Course> SqliteCourseRepository::findCourse(int id) {
    Statement stmt =
        db_.prepare("SELECT id, code, title, department_id FROM course WHERE id = ?");
    stmt.bind(1, id);
    if (!stmt.step()) return std::nullopt;
    return readCourse(stmt);
}

std::vector<Course> SqliteCourseRepository::allCourses() {
    Statement stmt =
        db_.prepare("SELECT id, code, title, department_id FROM course ORDER BY code");
    std::vector<Course> courses;
    while (stmt.step()) courses.push_back(readCourse(stmt));
    return courses;
}

void SqliteCourseRepository::addLecturer(int courseId, int lecturerId) {
    // OR IGNORE: assigning the same lecturer twice is not an error.
    Statement stmt = db_.prepare(
        "INSERT OR IGNORE INTO course_lecturer (course_id, lecturer_id) VALUES (?, ?)");
    stmt.bind(1, courseId).bind(2, lecturerId);
    stmt.step();
}

bool SqliteCourseRepository::isLecturerOf(int courseId, int lecturerId) {
    Statement stmt = db_.prepare(
        "SELECT 1 FROM course_lecturer WHERE course_id = ? AND lecturer_id = ?");
    stmt.bind(1, courseId).bind(2, lecturerId);
    return stmt.step();
}

void SqliteCourseRepository::addEnrolment(int courseId, int studentId) {
    Statement stmt = db_.prepare(
        "INSERT OR IGNORE INTO enrolment (course_id, student_id) VALUES (?, ?)");
    stmt.bind(1, courseId).bind(2, studentId);
    stmt.step();
}

void SqliteCourseRepository::removeEnrolment(int courseId, int studentId) {
    Statement stmt =
        db_.prepare("DELETE FROM enrolment WHERE course_id = ? AND student_id = ?");
    stmt.bind(1, courseId).bind(2, studentId);
    stmt.step();
}

std::vector<Course> SqliteCourseRepository::coursesForStudent(int studentId) {
    Statement stmt = db_.prepare(
        "SELECT c.id, c.code, c.title, c.department_id FROM course c "
        "JOIN enrolment e ON e.course_id = c.id WHERE e.student_id = ? ORDER BY c.code");
    stmt.bind(1, studentId);
    std::vector<Course> courses;
    while (stmt.step()) courses.push_back(readCourse(stmt));
    return courses;
}

Course SqliteCourseRepository::readCourse(const Statement& row) {
    return Course{row.columnInt(0), row.columnText(1), row.columnText(2), row.columnInt(3)};
}

}  // namespace lms
