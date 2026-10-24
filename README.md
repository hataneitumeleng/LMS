# Moderated LMS

![build](https://github.com/hataneitumeleng/LMS/actions/workflows/build.yml/badge.svg)

A console learning management system with late penalties, extensions and moderated marks, written in C++17 with SQLite (CS4433 individual project).

## Build guide (Windows)
1. Install MSYS2 and run this in **MSYS2 UCRT64**:
   ```
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-sqlite3 mingw-w64-ucrt-x86_64-libsodium mingw-w64-ucrt-x86_64-gtest
   ```
2. Put `C:\msys64\ucrt64\bin` at the **top** of the System Path. Other programs, such as SWI-Prolog or an old MinGW, ship DLLs with the same names and make the compiler crash if they come first.
3. Build and run:
   ```
   cmake -S . -B build -G Ninja
   cmake --build build
   .\build\lms.exe
   ```

## Build guide (Ubuntu 22.04)
```
sudo apt install g++ cmake ninja-build pkg-config libsqlite3-dev libsodium-dev libgtest-dev
cmake -S . -B build -G Ninja
cmake --build build
./build/lms
```

## Running the program
- Run `lms` from the project folder. It creates `lms.db` (the database) and a `submissions` folder next to it.
- **First start:** there are no users yet, so the program asks you to create the administrator account. Log in with it, then create the other users.
- An HoD is created by making a lecturer first, then choosing that lecturer when creating a department (Administrator menu, then 2, then 2).
- Dates are typed as `YYYY-MM-DD HH:MM`, for example `2026-11-30 17:00`.
- **Backup:** Administrator menu, then 4, then type a new folder name such as `backups/2026-11-30`. To restore, close the program and copy `lms.db` and `submissions` from the backup folder back into the project folder.

## Timing test (R12, NFR-01, NFR-02)
`lms_benchmark` fills `benchmark.db` with 800 students, 20 courses and 20,000 marked submissions, then times login and every list screen against the limits:
```
.\build\lms_benchmark.exe
```

## Automatic build
Every push to GitHub runs `.github/workflows/build.yml`. It builds the program on Ubuntu 22.04 and on Windows (MSYS2), runs a short start-up test, and runs the timing test (NFR-10).

## Project layout
| Folder | Layer (Week 3, Figure 1) |
|---|---|
| `src/ui` | Presentation: console menus, one per role |
| `src/services` | Business logic: AuthService, SubmissionService, ... |
| `src/data` | Data access: Database and the Sqlite repositories |
| `src/domain` | Domain classes: User, Submission, ... |
| `sql/schema.sql` | Database schema (Week 3, Appendix B) |
| `tests` | GoogleTest tests (Phase 5) |
| `tools` | `benchmark.cpp`, the R12 timing test |
| `docs` | Coding standard, traceability matrix (`RTM.md`) |
| `.github/workflows` | Automatic build on Ubuntu and Windows |
