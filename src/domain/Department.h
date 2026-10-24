#pragma once
#include <string>

namespace lms {

// A department, e.g. Science. Each department has one active HoD (Section 5.2).
struct Department {
    int id = 0;  // 0 until saved
    std::string name;
};

}  // namespace lms
