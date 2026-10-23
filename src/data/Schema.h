#pragma once

namespace lms {

// The contents of sql/schema.sql. CMake copies the file into the program when it
// builds (Schema.cpp.in), so the program is one .exe plus one database file (A7).
extern const char* const kSchemaSql;

}  // namespace lms
