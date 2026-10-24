#pragma once
#include "domain/Role.h"
#include "domain/TimeFormat.h"

#include <optional>
#include <string>

namespace lms {

// Abstract base class for the four kinds of user (class diagram, Figure 2).
// The subclasses differ in role(); the menus use role() to decide what to show (FR-02).
class User {
public:
    static constexpr int kMaxFailedAttempts = 5;  // NFR-04
    static constexpr int kLockMinutes = 5;        // NFR-04

    User(std::string username, std::string fullName);
    virtual ~User() = default;

    virtual Role role() const = 0;
    // Only a HeadOfDepartment has a department; the others return nothing.
    virtual std::optional<int> departmentId() const { return std::nullopt; }

    int id() const { return id_; }
    const std::string& username() const { return username_; }
    const std::string& fullName() const { return fullName_; }
    const std::string& passwordHash() const { return passwordHash_; }
    bool active() const { return active_; }
    int failedAttempts() const { return failedAttempts_; }
    const std::optional<TimePoint>& lockedUntil() const { return lockedUntil_; }

    void setId(int id) { id_ = id; }
    void setPasswordHash(const std::string& hash) { passwordHash_ = hash; }
    void setActive(bool active) { active_ = active; }
    void setLoginState(int failedAttempts, std::optional<TimePoint> lockedUntil);

    // NFR-04: locked for 5 minutes after 5 failed attempts in a row.
    bool isLocked(TimePoint now) const;
    void recordFailedLogin(TimePoint now);
    void recordSuccessfulLogin();

private:
    int id_ = 0;  // 0 until saved in the database
    std::string username_;
    std::string fullName_;
    std::string passwordHash_;
    bool active_ = true;
    int failedAttempts_ = 0;
    std::optional<TimePoint> lockedUntil_;
};

class Student : public User {
public:
    using User::User;
    Role role() const override { return Role::Student; }
};

class Lecturer : public User {
public:
    using User::User;
    Role role() const override { return Role::Lecturer; }
};

class HeadOfDepartment : public User {
public:
    HeadOfDepartment(std::string username, std::string fullName, int departmentId);
    Role role() const override { return Role::HoD; }
    std::optional<int> departmentId() const override { return departmentId_; }

private:
    int departmentId_;
};

class Administrator : public User {
public:
    using User::User;
    Role role() const override { return Role::Administrator; }
};

}  // namespace lms
