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

# ---- lms_core: domain + services + data layers ----
# Everything except the console menus goes into one library,
# so the GoogleTest tests (Phase 5) can link the same code.
add_library(lms_core STATIC
    src/domain/SubmissionStatus.cpp
)
target_include_directories(lms_core PUBLIC src)
target_link_libraries(lms_core PUBLIC ${LMS_SQLITE_TARGET} PkgConfig::SODIUM)

if(MSVC)
    target_compile_options(lms_core PRIVATE /W4)
else()
    target_compile_options(lms_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- lms: the console program (presentation layer) ----
add_executable(lms src/main.cpp)
target_link_libraries(lms PRIVATE lms_core)
