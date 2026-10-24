#include "data/SqliteAuditRepository.h"

namespace lms {

namespace {
const char* const kEntryColumns =
    "SELECT a.id, a.submission_id, a.user_id, a.changed_at, a.old_value, a.new_value, a.reason "
    "FROM audit_entry a ";
}  // namespace

SqliteAuditRepository::SqliteAuditRepository(Database& db) : db_(db) {}

void SqliteAuditRepository::add(const AuditEntry& entry) {
    Statement stmt = db_.prepare(
        "INSERT INTO audit_entry (submission_id, user_id, changed_at, old_value, new_value, "
        "reason) VALUES (?, ?, ?, ?, ?, ?)");
    stmt.bind(1, entry.submissionId).bind(2, entry.userId).bind(3, toText(entry.changedAt));
    if (entry.oldValue) {
        stmt.bind(4, *entry.oldValue);
    } else {
        stmt.bindNull(4);
    }
    stmt.bind(5, entry.newValue).bind(6, entry.reason);
    stmt.step();
}

std::vector<AuditEntry> SqliteAuditRepository::findByDepartment(int departmentId) {
    // Follow the links audit_entry -> submission -> assignment -> course to the department.
    Statement stmt = db_.prepare(std::string(kEntryColumns) +
                                 "JOIN submission s ON s.id = a.submission_id "
                                 "JOIN assignment asg ON asg.id = s.assignment_id "
                                 "JOIN course c ON c.id = asg.course_id "
                                 "WHERE c.department_id = ? ORDER BY a.id");
    stmt.bind(1, departmentId);
    std::vector<AuditEntry> entries;
    while (stmt.step()) entries.push_back(readEntry(stmt));
    return entries;
}

std::vector<AuditEntry> SqliteAuditRepository::findAll() {
    Statement stmt = db_.prepare(std::string(kEntryColumns) + "ORDER BY a.id");
    std::vector<AuditEntry> entries;
    while (stmt.step()) entries.push_back(readEntry(stmt));
    return entries;
}

AuditEntry SqliteAuditRepository::readEntry(const Statement& row) {
    AuditEntry entry;
    entry.id = row.columnInt(0);
    entry.submissionId = row.columnInt(1);
    entry.userId = row.columnInt(2);
    entry.changedAt = timeFromText(row.columnText(3));
    if (!row.columnIsNull(4)) entry.oldValue = row.columnDouble(4);
    entry.newValue = row.columnDouble(5);
    entry.reason = row.columnText(6);
    return entry;
}

}  // namespace lms
