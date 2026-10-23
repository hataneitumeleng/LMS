# Moderated LMS

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

## Project layout
| Folder | Layer (Week 3, Figure 1) |
|---|---|
| `src/ui` | Presentation: console menus, one per role |
| `src/services` | Business logic: AuthService, SubmissionService, ... |
| `src/data` | Data access: Database and the Sqlite repositories |
| `src/domain` | Domain classes: User, Submission, ... |
| `sql/schema.sql` | Database schema (Week 3, Appendix B) |
| `tests` | GoogleTest tests (Phase 5) |
| `docs` | Coding standard |
