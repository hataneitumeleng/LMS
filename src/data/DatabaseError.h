#pragma once
#include <stdexcept>
#include <string>

namespace lms {

// Thrown when SQLite reports an error. The menus catch it and show a message (NFR-07).
class DatabaseError : public std::runtime_error {
public:
    explicit DatabaseError(const std::string& message) : std::runtime_error(message) {}
};

}  // namespace lms
