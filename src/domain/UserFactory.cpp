#include "domain/UserFactory.h"

#include "domain/Errors.h"

namespace lms {

std::unique_ptr<User> UserFactory::create(Role role, const std::string& username,
                                          const std::string& fullName,
                                          std::optional<int> departmentId) const {
    if (username.empty()) throw ValidationError("Username must not be empty");
    if (fullName.empty()) throw ValidationError("Full name must not be empty");

    if (role == Role::HoD) {
        if (!departmentId) throw ValidationError("An HoD must have a department");
        return std::make_unique<HeadOfDepartment>(username, fullName, *departmentId);
    }
    if (departmentId) throw ValidationError("Only an HoD can have a department");

    switch (role) {
        case Role::Student:       return std::make_unique<Student>(username, fullName);
        case Role::Lecturer:      return std::make_unique<Lecturer>(username, fullName);
        case Role::Administrator: return std::make_unique<Administrator>(username, fullName);
        case Role::HoD:           break;  // handled above
    }
    throw ValidationError("Unknown role");
}

}  // namespace lms
