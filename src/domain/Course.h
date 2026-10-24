#pragma once
#include <string>

namespace lms {

// A course belongs to exactly one department (FR-04).
struct Course {
    int id = 0;  // 0 until saved
    std::string code;
    std::string title;
    int departmentId = 0;
};

}  // namespace lms
