#include "services/AuthService.h"

#include "domain/Errors.h"

namespace lms {

namespace {
// The same message for "no such user" and "wrong password", so an attacker
// cannot find out which usernames exist.
const char* const kLoginFailed = "Invalid username or password";
}  // namespace

AuthService::AuthService(IUserRepository& users, const IPasswordHasher& hasher,
                         const IClock& clock)
    : users_(users), hasher_(hasher), clock_(clock) {}

std::unique_ptr<User> AuthService::login(const std::string& username,
                                         const std::string& password) {
    std::unique_ptr<User> user = users_.findByUsername(username);
    if (!user || !user->active()) {
        throw AuthError(kLoginFailed);
    }

    const TimePoint now = clock_.now();
    if (user->isLocked(now)) {
        throw AuthError("Account locked until " + toText(*user->lockedUntil()));
    }

    if (!hasher_.verify(password, user->passwordHash())) {
        user->recordFailedLogin(now);  // may lock the account (NFR-04)
        users_.save(*user);
        throw AuthError(kLoginFailed);
    }

    user->recordSuccessfulLogin();
    users_.save(*user);
    return user;
}

void AuthService::changePassword(int userId, const std::string& oldPassword,
                                 const std::string& newPassword) {
    std::unique_ptr<User> user = users_.findById(userId);
    if (!user || !hasher_.verify(oldPassword, user->passwordHash())) {
        throw AuthError("Current password is wrong");
    }
    checkPasswordRules(newPassword);
    user->setPasswordHash(hasher_.hash(newPassword));
    users_.save(*user);
}

void AuthService::checkPasswordRules(const std::string& password) {
    if (password.size() < kMinPasswordLength) {
        throw ValidationError("Password must be at least 8 characters");
    }
}

}  // namespace lms
