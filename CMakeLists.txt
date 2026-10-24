cmake_minimum_required(VERSION 3.16)
project(ModeratedLMS VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Libraries (same lines as the R11 build test) ----
find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

# New CMake versions call the SQLite target SQLite3::SQLite3, older ones SQLite::SQLite3.
if(TARGET SQLite3::SQLite3)
    set(LMS_SQLITE_TARGET SQLite3::SQLite3)
else()
    set(LMS_SQLITE_TARGET SQLite::SQLite3)
endif()

# ---- Copy sql/schema.sql into the program as a C++ string ----
file(READ ${CMAKE_SOURCE_DIR}/sql/schema.sql LMS_SCHEMA_SQL)
configure_file(src/data/Schema.cpp.in ${CMAKE_BINARY_DIR}/generated/Schema.cpp @ONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS sql/schema.sql)

# ---- lms_core: domain + services + data layers ----
# Everything except the console menus goes into one library,
# so the GoogleTest tests (Phase 5) can link the same code.
add_library(lms_core STATIC
    src/domain/SubmissionStatus.cpp
    src/domain/Role.cpp
    src/domain/TimeFormat.cpp
    src/domain/User.cpp
    src/domain/UserFactory.cpp
    src/domain/Submission.cpp
    src/data/Statement.cpp
    src/data/Database.cpp
    src/data/SqliteUserRepository.cpp
    src/data/SqliteCourseRepository.cpp
    src/data/SqliteAssignmentRepository.cpp
    src/data/SqliteSubmissionRepository.cpp
    src/data/SqliteAuditRepository.cpp
    src/services/Argon2Hasher.cpp
    src/services/AuthService.cpp
    src/services/AdminService.cpp
    src/services/TenPercentPerDayPolicy.cpp
    src/services/AssignmentService.cpp
    src/services/FileStore.cpp
    src/services/SubmissionService.cpp
    src/services/AuditService.cpp
    src/services/MarkingService.cpp
    src/services/ModerationService.cpp
    src/services/BackupService.cpp
    ${CMAKE_BINARY_DIR}/generated/Schema.cpp
)
target_include_directories(lms_core PUBLIC src)
target_link_libraries(lms_core PUBLIC ${LMS_SQLITE_TARGET} PkgConfig::SODIUM)

if(MSVC)
    target_compile_options(lms_core PRIVATE /W4)
else()
    target_compile_options(lms_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- lms: the console program (presentation layer) ----
add_executable(lms
    src/main.cpp
    src/ui/Console.cpp
    src/ui/Menu.cpp
    src/ui/LoginMenu.cpp
    src/ui/StudentMenu.cpp
    src/ui/LecturerMenu.cpp
    src/ui/HodMenu.cpp
    src/ui/AdminMenu.cpp
    src/ui/App.cpp
)
target_link_libraries(lms PRIVATE lms_core)
if(NOT MSVC)
    target_compile_options(lms PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- lms_benchmark: R12 timing test (NFR-01, NFR-02) ----
add_executable(lms_benchmark tools/benchmark.cpp)
target_link_libraries(lms_benchmark PRIVATE lms_core)
