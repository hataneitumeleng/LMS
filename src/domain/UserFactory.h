#pragma once
#include "domain/Role.h"
#include "domain/User.h"

#include <memory>
#include <optional>
#include <string>

namespace lms {

// Factory pattern (Table 1): creates the right User subclass for a role, and checks
// that an HoD has a department and nobody else does. Used by AdminService (FR-03)
// and by SqliteUserRepository when it reads a user row.
class UserFactory {
public:
    std::unique_ptr<User> create(Role role, const std::string& username,
                                 const std::string& fullName,
                                 std::optional<int> departmentId) const;
};

}  // namespace lms
