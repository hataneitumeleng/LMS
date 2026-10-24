#include "data/SqliteUserRepository.h"

namespace lms {

namespace {
// The same column list is used by every SELECT so readUser() can rely on the order.
const char* const kUserColumns =
    "SELECT id, username, password_hash, full_name, role, department_id, active, "
    "failed_attempts, locked_until FROM user ";
}  // namespace

SqliteUserRepository::SqliteUserRepository(Database& db, const UserFactory& factory)
    : db_(db), factory_(factory) {}

std::unique_ptr<User> SqliteUserRepository::findByUsername(const std::string& username) {
    Statement stmt = db_.prepare(std::string(kUserColumns) + "WHERE username = ?");
    stmt.bind(1, username);
    return stmt.step() ? readUser(stmt) : nullptr;
}

std::unique_ptr<User> SqliteUserRepository::findById(int id) {
    Statement stmt = db_.prepare(std::string(kUserColumns) + "WHERE id = ?");
    stmt.bind(1, id);
    return stmt.step() ? readUser(stmt) : nullptr;
}

std::unique_ptr<User> SqliteUserRepository::findActiveHod(int departmentId) {
    Statement stmt = db_.prepare(std::string(kUserColumns) +
                                 "WHERE role = 'HoD' AND active = 1 AND department_id = ?");
    stmt.bind(1, departmentId);
    return stmt.step() ? readUser(stmt) : nullptr;
}

std::vector<std::unique_ptr<User>> SqliteUserRepository::findAll() {
    Statement stmt = db_.prepare(std::string(kUserColumns) + "ORDER BY role, username");
    std::vector<std::unique_ptr<User>> users;
    while (stmt.step()) users.push_back(readUser(stmt));
    return users;
}

int SqliteUserRepository::save(const User& user) {
    const bool isNew = user.id() == 0;
    Statement stmt = db_.prepare(
        isNew ? "INSERT INTO user (username, password_hash, full_name, role, department_id, "
                "active, failed_attempts, locked_until) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
              : "UPDATE user SET username = ?, password_hash = ?, full_name = ?, role = ?, "
                "department_id = ?, active = ?, failed_attempts = ?, locked_until = ? "
                "WHERE id = ?");
    stmt.bind(1, user.username());
    stmt.bind(2, user.passwordHash());
    stmt.bind(3, user.fullName());
    stmt.bind(4, toString(user.role()));
    if (user.departmentId()) {
        stmt.bind(5, *user.departmentId());
    } else {
        stmt.bindNull(5);
    }
    stmt.bind(6, user.active() ? 1 : 0);
    stmt.bind(7, user.failedAttempts());
    if (user.lockedUntil()) {
        stmt.bind(8, toText(*user.lockedUntil()));
    } else {
        stmt.bindNull(8);
    }
    if (!isNew) stmt.bind(9, user.id());
    stmt.step();
    return isNew ? static_cast<int>(db_.lastInsertId()) : user.id();
}

std::unique_ptr<User> SqliteUserRepository::readUser(Statement& row) const {
    std::optional<int> departmentId;
    if (!row.columnIsNull(5)) departmentId = row.columnInt(5);

    // The factory builds the right subclass from the role column.
    std::unique_ptr<User> user = factory_.create(roleFromString(row.columnText(4)),
                                                 row.columnText(1), row.columnText(3),
                                                 departmentId);
    user->setId(row.columnInt(0));
    user->setPasswordHash(row.columnText(2));
    user->setActive(row.columnInt(6) == 1);

    std::optional<TimePoint> lockedUntil;
    if (!row.columnIsNull(8)) lockedUntil = timeFromText(row.columnText(8));
    user->setLoginState(row.columnInt(7), lockedUntil);
    return user;
}

}  // namespace lms
