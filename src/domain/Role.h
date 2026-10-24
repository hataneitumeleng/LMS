#pragma once
#include <string>

namespace lms {

// The four roles from Week 2. The text matches the CHECK on user.role.
enum class Role { Student, Lecturer, HoD, Administrator };

std::string toString(Role role);

// Throws ValidationError for unknown text.
Role roleFromString(const std::string& text);

}  // namespace lms
