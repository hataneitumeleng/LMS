#include "domain/User.h"

#include <utility>

namespace lms {

User::User(std::string username, std::string fullName)
    : username_(std::move(username)), fullName_(std::move(fullName)) {}

void User::setLoginState(int failedAttempts, std::optional<TimePoint> lockedUntil) {
    failedAttempts_ = failedAttempts;
    lockedUntil_ = lockedUntil;
}

bool User::isLocked(TimePoint now) const {
    return lockedUntil_.has_value() && now < *lockedUntil_;
}

void User::recordFailedLogin(TimePoint now) {
    ++failedAttempts_;
    if (failedAttempts_ >= kMaxFailedAttempts) {
        lockedUntil_ = now + std::chrono::minutes(kLockMinutes);
        failedAttempts_ = 0;  // a new count starts after the lock ends
    }
}

void User::recordSuccessfulLogin() {
    failedAttempts_ = 0;
    lockedUntil_.reset();
}

HeadOfDepartment::HeadOfDepartment(std::string username, std::string fullName, int departmentId)
    : User(std::move(username), std::move(fullName)), departmentId_(departmentId) {}

}  // namespace lms
