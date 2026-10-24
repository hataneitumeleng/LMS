#pragma once
#include "data/IUserRepository.h"
#include "services/IClock.h"
#include "services/IPasswordHasher.h"

#include <memory>
#include <string>

namespace lms {

// Login and password change (UC-01, UC-15; FR-01, FR-19, NFR-03, NFR-04).
class AuthService {
public:
    static constexpr std::size_t kMinPasswordLength = 8;  // NFR-04

    AuthService(IUserRepository& users, const IPasswordHasher& hasher, const IClock& clock);

    // Returns the logged-in user. Throws AuthError if login is refused.
    std::unique_ptr<User> login(const std::string& username, const std::string& password);

    // Throws AuthError if the old password is wrong, ValidationError if the new one is too short.
    void changePassword(int userId, const std::string& oldPassword,
                        const std::string& newPassword);

    // Throws ValidationError if the password breaks NFR-04. Also used by AdminService.
    static void checkPasswordRules(const std::string& password);

private:
    IUserRepository& users_;
    const IPasswordHasher& hasher_;
    const IClock& clock_;
};

}  // namespace lms
