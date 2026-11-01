# Moderated LMS

![build](https://github.com/hataneitumeleng/LMS/actions/workflows/build.yml/badge.svg)

A learning management system with late penalties, extensions and moderated marks, written in C++17 with SQLite (CS4433 individual project). It has two front ends on the same data: a console program (`lms`) and a Qt desktop program (`lms_gui`).

## Build guide (Windows)
1. Install MSYS2 and run this in **MSYS2 UCRT64**:
   ```
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-sqlite3 mingw-w64-ucrt-x86_64-libsodium mingw-w64-ucrt-x86_64-gtest mingw-w64-ucrt-x86_64-qt6-base
   ```
   `qt6-base` is only needed for the desktop program. Without it, CMake skips `lms_gui` and builds the console program.
2. Put `C:\msys64\ucrt64\bin` at the **top** of the System Path. Other programs, such as SWI-Prolog or an old MinGW, ship DLLs with the same names and make the compiler crash if they come first.
3. Build and run:
   ```
   cmake -S . -B build -G Ninja
   cmake --build build
   .\build\lms.exe       (console)
   .\build\lms_gui.exe   (desktop)
   ```

## Build guide (Ubuntu 22.04)
```
sudo apt install g++ cmake ninja-build pkg-config libsqlite3-dev libsodium-dev libgtest-dev qt6-base-dev libgl1-mesa-dev
cmake -S . -B build -G Ninja
cmake --build build
./build/lms        # console
./build/lms_gui    # desktop
```

## Running the program
- Run `lms` from the project folder. It creates `lms.db` (the database) and a `submissions` folder next to it.
- **First start:** there are no users yet, so the program asks you to create the administrator account. Log in with it, then create the other users.
- An HoD is created by making a lecturer first, then choosing that lecturer when creating a department (Administrator menu, then 2, then 2).
- Dates are typed as `YYYY-MM-DD HH:MM`, for example `2026-11-30 17:00`.
- **Backup:** Administrator menu, then 4, then type a new folder name such as `backups/2026-11-30`. To restore, close the program and copy `lms.db` and `submissions` from the backup folder back into the project folder.

## The desktop program
- Run `lms_gui` from the project folder. It uses the same `lms.db` and `submissions` folder as `lms`.
- Log in, and your role's window opens with one tab per task. The **Account** menu has "Change my password" and "Log out".
- Choose a row in a list (an assignment, a submission, a user), then press the button for what you want to do.
- Errors are shown in a message box. Release, approve all, deactivate, replace HoD and remove a student ask for confirmation first.

## Tests (Phase 5)
GoogleTest unit tests are in `tests/`. They build into `lms_tests` when GoogleTest is installed (`mingw-w64-ucrt-x86_64-gtest` or `libgtest-dev`). Run them all with:
```
ctest --test-dir build --output-on-failure
```
They use a fake clock (`tests/fakes/FakeClock.h`), so they give the same result on any day.

| Folder | Kind of test |
|---|---|
| `tests/unit` | Unit tests: one class at a time (penalty, Submission states, User, Argon2) |
| `tests/service` | Service tests: each service with in-memory fake repositories (`tests/fakes`) |
| `tests/integration` | Integration tests: the real SQLite repositories and services on a real database file |
| `tests/system` | System tests: typed input into the real `lms` program, checked against `.expect` files |

Only the system tests: `ctest --test-dir build -R "^System[.]"`. The test cases and results are in `docs/SYSTEM_TESTS.md`, the defects in `docs/DEFECT_LOG.md`.

**Coverage.** Configure a second build with `-DLMS_COVERAGE=ON` and run gcovr. GitHub Actions does this in the `coverage` job, which fails below 70% for the business layer and puts the report on the run's summary page.

## Timing test (R12, NFR-01, NFR-02)
`lms_benchmark` fills `benchmark.db` with 800 students, 20 courses and 20,000 marked submissions, then times login and every list screen against the limits:
```
.\build\lms_benchmark.exe
```

## Automatic build
Every push to GitHub runs `.github/workflows/build.yml`. It builds the program on Ubuntu 22.04 and on Windows (MSYS2), runs all the tests (unit, service, integration and system), a short start-up test and the timing test (NFR-10), and measures test coverage.

## Project layout
| Folder | Layer (Week 3, Figure 1) |
|---|---|
| `src/ui` | Presentation: console menus, one per role |
| `src/gui` | Presentation: Qt desktop windows, one per role |
| `src/app` | `System`, the composition root both programs start from |
| `src/services` | Business logic: AuthService, SubmissionService, ... |
| `src/data` | Data access: Database and the Sqlite repositories |
| `src/domain` | Domain classes: User, Submission, ... |
| `sql/schema.sql` | Database schema (Week 3, Appendix B) |
| `tests` | GoogleTest tests: `unit/` (one file per class), `fakes/` (fake clock) |
| `tools` | `benchmark.cpp`, the R12 timing test |
| `docs` | Coding standard, traceability matrix (`RTM.md`) |
| `.github/workflows` | Automatic build on Ubuntu and Windows |
