#include "data/SqliteAssignmentRepository.h"

namespace lms {

namespace {
const char* const kAssignmentColumns =
    "SELECT id, course_id, title, coalesce(description, ''), deadline, max_mark, created_by, "
    "coalesce(brief_path, '') FROM assignment ";
}  // namespace

SqliteAssignmentRepository::SqliteAssignmentRepository(Database& db) : db_(db) {}

std::optional<Assignment> SqliteAssignmentRepository::find(int id) {
    Statement stmt = db_.prepare(std::string(kAssignmentColumns) + "WHERE id = ?");
    stmt.bind(1, id);
    if (!stmt.step()) return std::nullopt;
    return readAssignment(stmt);
}

std::vector<Assignment> SqliteAssignmentRepository::forCourse(int courseId) {
    Statement stmt =
        db_.prepare(std::string(kAssignmentColumns) + "WHERE course_id = ? ORDER BY deadline");
    stmt.bind(1, courseId);
    std::vector<Assignment> assignments;
    while (stmt.step()) assignments.push_back(readAssignment(stmt));
    return assignments;
}

int SqliteAssignmentRepository::save(const Assignment& assignment) {
    if (assignment.id == 0) {
        Statement stmt = db_.prepare(
            "INSERT INTO assignment (course_id, title, description, deadline, max_mark, "
            "created_by, brief_path) VALUES (?, ?, ?, ?, ?, ?, ?)");
        stmt.bind(1, assignment.courseId)
            .bind(2, assignment.title)
            .bind(3, assignment.description)
            .bind(4, toText(assignment.deadline))
            .bind(5, assignment.maxMark)
            .bind(6, assignment.createdBy);
        bindBrief(stmt, 7, assignment.briefPath);
        stmt.step();
        return static_cast<int>(db_.lastInsertId());
    }
    Statement stmt = db_.prepare(
        "UPDATE assignment SET title = ?, description = ?, deadline = ?, max_mark = ?, "
        "brief_path = ? WHERE id = ?");
    stmt.bind(1, assignment.title)
        .bind(2, assignment.description)
        .bind(3, toText(assignment.deadline))
        .bind(4, assignment.maxMark);
    bindBrief(stmt, 5, assignment.briefPath);
    stmt.bind(6, assignment.id);
    stmt.step();
    return assignment.id;
}

std::optional<Extension> SqliteAssignmentRepository::findExtension(int assignmentId,
                                                                   int studentId) {
    Statement stmt = db_.prepare(
        "SELECT new_deadline, reason, granted_by, granted_at FROM extension "
        "WHERE assignment_id = ? AND student_id = ?");
    stmt.bind(1, assignmentId).bind(2, studentId);
    if (!stmt.step()) return std::nullopt;
    return Extension{assignmentId,
                     studentId,
                     timeFromText(stmt.columnText(0)),
                     stmt.columnText(1),
                     stmt.columnInt(2),
                     timeFromText(stmt.columnText(3))};
}

void SqliteAssignmentRepository::saveExtension(const Extension& extension) {
    // OR REPLACE: the primary key (assignment_id, student_id) allows one extension
    // per student per assignment, so a new one replaces the old one.
    Statement stmt = db_.prepare(
        "INSERT OR REPLACE INTO extension "
        "(assignment_id, student_id, new_deadline, reason, granted_by, granted_at) "
        "VALUES (?, ?, ?, ?, ?, ?)");
    stmt.bind(1, extension.assignmentId)
        .bind(2, extension.studentId)
        .bind(3, toText(extension.newDeadline))
        .bind(4, extension.reason)
        .bind(5, extension.grantedBy)
        .bind(6, toText(extension.grantedAt));
    stmt.step();
}

std::vector<std::string> SqliteAssignmentRepository::allBriefPaths() {
    Statement stmt = db_.prepare("SELECT brief_path FROM assignment WHERE brief_path IS NOT NULL");
    std::vector<std::string> paths;
    while (stmt.step()) paths.push_back(stmt.columnText(0));
    return paths;
}

Assignment SqliteAssignmentRepository::readAssignment(const Statement& row) {
    return Assignment{row.columnInt(0),  row.columnInt(1),
                      row.columnText(2), row.columnText(3),
                      timeFromText(row.columnText(4)), row.columnInt(5),
                      row.columnInt(6),  row.columnText(7)};
}

void SqliteAssignmentRepository::bindBrief(Statement& stmt, int index, const std::string& path) {
    if (path.empty()) {
        stmt.bindNull(index);  // no brief is stored as NULL, not as ''
    } else {
        stmt.bind(index, path);
    }
}

}  // namespace lms
