#pragma once
#include <stdexcept>
#include <string>

namespace lms {

// Thrown when a value typed by a user breaks a rule (e.g. password too short).
class ValidationError : public std::runtime_error {
public:
    explicit ValidationError(const std::string& message) : std::runtime_error(message) {}
};

// Thrown when login fails or a user is not allowed to do something (FR-01, FR-02).
class AuthError : public std::runtime_error {
public:
    explicit AuthError(const std::string& message) : std::runtime_error(message) {}
};

}  // namespace lms
