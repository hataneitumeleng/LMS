#include "domain/Role.h"

#include "domain/Errors.h"

namespace lms {

std::string toString(Role role) {
    switch (role) {
        case Role::Student:       return "Student";
        case Role::Lecturer:      return "Lecturer";
        case Role::HoD:           return "HoD";
        case Role::Administrator: return "Administrator";
    }
    throw ValidationError("Unknown role");
}

Role roleFromString(const std::string& text) {
    if (text == "Student")       return Role::Student;
    if (text == "Lecturer")      return Role::Lecturer;
    if (text == "HoD")           return Role::HoD;
    if (text == "Administrator") return Role::Administrator;
    throw ValidationError("Unknown role: " + text);
}

}  // namespace lms
