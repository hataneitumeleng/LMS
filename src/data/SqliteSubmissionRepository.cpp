#include "data/SqliteSubmissionRepository.h"

namespace lms {

namespace {
const char* const kSubmissionColumns =
    "SELECT id, assignment_id, student_id, status, days_late, raw_mark, final_mark, "
    "coalesce(feedback, ''), marked_by, moderated_by, moderated_at FROM submission ";

// Binds an optional value, or NULL when it is empty.
template <typename T>
void bindOptional(Statement& stmt, int index, const std::optional<T>& value) {
    if (value) {
        stmt.bind(index, *value);
    } else {
        stmt.bindNull(index);
    }
}
}  // namespace

SqliteSubmissionRepository::SqliteSubmissionRepository(Database& db) : db_(db) {}

std::optional<Submission> SqliteSubmissionRepository::findCurrent(int studentId,
                                                                  int assignmentId) {
    Statement stmt = db_.prepare(std::string(kSubmissionColumns) +
                                 "WHERE student_id = ? AND assignment_id = ?");
    stmt.bind(1, studentId).bind(2, assignmentId);
    if (!stmt.step()) return std::nullopt;
    return readSubmission(stmt);
}

std::optional<Submission> SqliteSubmissionRepository::findById(int id) {
    Statement stmt = db_.prepare(std::string(kSubmissionColumns) + "WHERE id = ?");
    stmt.bind(1, id);
    if (!stmt.step()) return std::nullopt;
    return readSubmission(stmt);
}

std::vector<Submission> SqliteSubmissionRepository::findByAssignment(int assignmentId) {
    Statement stmt = db_.prepare(std::string(kSubmissionColumns) +
                                 "WHERE assignment_id = ? ORDER BY student_id");
    stmt.bind(1, assignmentId);
    std::vector<Submission> submissions;
    while (stmt.step()) submissions.push_back(readSubmission(stmt));
    return submissions;
}

int SqliteSubmissionRepository::save(const Submission& s) {
    std::optional<std::string> moderatedAt;
    if (s.moderatedAt) moderatedAt = toText(*s.moderatedAt);

    if (s.id == 0) {
        Statement stmt = db_.prepare(
            "INSERT INTO submission (assignment_id, student_id, status, days_late, raw_mark, "
            "final_mark, feedback, marked_by, moderated_by, moderated_at) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        stmt.bind(1, s.assignmentId).bind(2, s.studentId).bind(3, toString(s.status));
        stmt.bind(4, s.daysLate);
        bindOptional(stmt, 5, s.rawMark);
        bindOptional(stmt, 6, s.finalMark);
        stmt.bind(7, s.feedback);
        bindOptional(stmt, 8, s.markedBy);
        bindOptional(stmt, 9, s.moderatedBy);
        bindOptional(stmt, 10, moderatedAt);
        stmt.step();
        return static_cast<int>(db_.lastInsertId());
    }
    Statement stmt = db_.prepare(
        "UPDATE submission SET status = ?, days_late = ?, raw_mark = ?, final_mark = ?, "
        "feedback = ?, marked_by = ?, moderated_by = ?, moderated_at = ? WHERE id = ?");
    stmt.bind(1, toString(s.status)).bind(2, s.daysLate);
    bindOptional(stmt, 3, s.rawMark);
    bindOptional(stmt, 4, s.finalMark);
    stmt.bind(5, s.feedback);
    bindOptional(stmt, 6, s.markedBy);
    bindOptional(stmt, 7, s.moderatedBy);
    bindOptional(stmt, 8, moderatedAt);
    stmt.bind(9, s.id);
    stmt.step();
    return s.id;
}

void SqliteSubmissionRepository::addVersion(int submissionId, const SubmissionVersion& version) {
    Statement stmt = db_.prepare(
        "INSERT INTO submission_version (submission_id, version_no, file_path, submitted_at) "
        "VALUES (?, ?, ?, ?)");
    stmt.bind(1, submissionId)
        .bind(2, version.versionNo)
        .bind(3, version.filePath)
        .bind(4, toText(version.submittedAt));
    stmt.step();
}

std::optional<SubmissionVersion> SqliteSubmissionRepository::latestVersion(int submissionId) {
    Statement stmt = db_.prepare(
        "SELECT version_no, file_path, submitted_at FROM submission_version "
        "WHERE submission_id = ? ORDER BY version_no DESC LIMIT 1");
    stmt.bind(1, submissionId);
    if (!stmt.step()) return std::nullopt;
    return readVersion(stmt);
}

std::vector<SubmissionVersion> SqliteSubmissionRepository::versions(int submissionId) {
    Statement stmt = db_.prepare(
        "SELECT version_no, file_path, submitted_at FROM submission_version "
        "WHERE submission_id = ? ORDER BY version_no");
    stmt.bind(1, submissionId);
    std::vector<SubmissionVersion> result;
    while (stmt.step()) result.push_back(readVersion(stmt));
    return result;
}

std::vector<std::string> SqliteSubmissionRepository::allFilePaths() {
    Statement stmt = db_.prepare("SELECT file_path FROM submission_version");
    std::vector<std::string> paths;
    while (stmt.step()) paths.push_back(stmt.columnText(0));
    return paths;
}

Submission SqliteSubmissionRepository::readSubmission(const Statement& row) {
    Submission s;
    s.id = row.columnInt(0);
    s.assignmentId = row.columnInt(1);
    s.studentId = row.columnInt(2);
    s.status = submissionStatusFromString(row.columnText(3));
    s.daysLate = row.columnInt(4);
    if (!row.columnIsNull(5)) s.rawMark = row.columnDouble(5);
    if (!row.columnIsNull(6)) s.finalMark = row.columnDouble(6);
    s.feedback = row.columnText(7);
    if (!row.columnIsNull(8)) s.markedBy = row.columnInt(8);
    if (!row.columnIsNull(9)) s.moderatedBy = row.columnInt(9);
    if (!row.columnIsNull(10)) s.moderatedAt = timeFromText(row.columnText(10));
    return s;
}

SubmissionVersion SqliteSubmissionRepository::readVersion(const Statement& row) {
    return SubmissionVersion{row.columnInt(0), row.columnText(1),
                             timeFromText(row.columnText(2))};
}

}  // namespace lms
