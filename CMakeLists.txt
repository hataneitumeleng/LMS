cmake_minimum_required(VERSION 3.16)
project(ModeratedLMS VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Test coverage (Phase 5): cmake -S . -B build-coverage -G Ninja -DLMS_COVERAGE=ON ----
# Adds gcc's --coverage flags to every target, so running the tests writes .gcda files that
# gcovr turns into a report. Off by default: a normal build is not slowed down.
option(LMS_COVERAGE "Build with gcc coverage instrumentation" OFF)
if(LMS_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

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
# Everything except the console menus and the Qt windows goes into one library, so both
# programs and the GoogleTest tests (Phase 5) link the same code. src/app/System.cpp is the
# composition root that both programs start from.
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
    src/app/System.cpp
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

# ---- lms_tests: GoogleTest unit and integration tests (Phase 5, NFR-09) ----
# Built only when GoogleTest is installed. Run them with:  ctest --test-dir build
enable_testing()
find_package(GTest QUIET)
if(GTest_FOUND)
    add_executable(lms_tests
        tests/unit/PenaltyPolicyTest.cpp
        tests/unit/SubmissionTest.cpp
        tests/unit/UserTest.cpp
        tests/unit/Argon2HasherTest.cpp
        tests/service/AuthServiceTest.cpp
        tests/service/AssignmentServiceTest.cpp
        tests/service/SubmissionServiceTest.cpp
        tests/service/MarkingServiceTest.cpp
        tests/service/ModerationServiceTest.cpp
        tests/service/AuditServiceTest.cpp
        tests/service/AdminServiceTest.cpp
        tests/integration/SqliteRepositoryTest.cpp
        tests/integration/SqliteServiceTest.cpp
    )
    target_link_libraries(lms_tests PRIVATE lms_core GTest::gtest GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(lms_tests)
    message(STATUS "GoogleTest found: building lms_tests")
else()
    message(STATUS "GoogleTest not found: lms_tests is skipped")
endif()

# ---- lms_gui: the Qt desktop program (a second presentation layer) ----
# Built only when Qt 6 Widgets is installed, so the console program still builds without it.
find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
    add_executable(lms_gui
        src/main_gui.cpp
        src/gui/GuiCommon.cpp
        src/gui/LoginDialog.cpp
        src/gui/ChangePasswordDialog.cpp
        src/gui/RoleWindow.cpp
        src/gui/StudentWindow.cpp
        src/gui/LecturerWindow.cpp
        src/gui/HodWindow.cpp
        src/gui/AdminWindow.cpp
        src/gui/GuiApp.cpp
    )
    target_link_libraries(lms_gui PRIVATE lms_core Qt6::Widgets)
    # On Windows: a normal window program, without a black console window behind it.
    set_target_properties(lms_gui PROPERTIES WIN32_EXECUTABLE ON)
    if(NOT MSVC)
        target_compile_options(lms_gui PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    message(STATUS "Qt ${Qt6_VERSION} found: building lms_gui")
else()
    message(STATUS "Qt 6 not found: lms_gui is skipped (the console program lms is built)")
endif()
