# Coding Standard: Moderated LMS

This project follows a small subset of the C++ Core Guidelines (Stroustrup and Sutter), adapted to the Week 3 design.

## 1. Files and layout
- One class per header (`.h`) and source (`.cpp`) file, named after the class: `SubmissionService.h` and `SubmissionService.cpp`.
- Folders follow the layers in Figure 1:
  - `src/ui` holds the menus;
  - `src/services` holds the business logic;
  - `src/data` holds the repositories and Database;
  - `src/domain` holds the domain classes.
- A file may only include files from its own layer or a lower one: ui → services → data → domain. The menus never include a repository.
- Headers start with `#pragma once`. All code is inside `namespace lms`.

## 2. Naming
| Item | Style | Example |
|---|---|---|
| Classes, enums, structs | PascalCase | `MarkingService`, `SubmissionStatus` |
| Interfaces | `I` + PascalCase | `IPenaltyPolicy`, `IUserRepository` |
| Functions and variables | camelCase | `grantExtension()`, `daysLate` |
| Private data members | camelCase + `_` | `repository_`, `clock_` |
| Constants | `k` + PascalCase | `kMaxExtensionDays` |
| Database tables and columns | snake_case, as in `schema.sql` | `audit_entry.old_value` |

Class, method and table names must match the Week 3 class diagrams and ERD exactly.

## 3. Formatting
- 4 spaces, no tabs. Lines of 100 characters or fewer.
- The opening brace goes on the same line. Always use braces, even for one-line `if` bodies.

## 4. Safety rules
- **SQL:** every query uses a prepared statement with `sqlite3_bind_*`. User input is never added to an SQL string (security, FR-01).
- **Passwords:** stored only as Argon2id hashes from libsodium. Plain passwords are never written to a file, the database or the screen.
- **Memory:** no `new`/`delete` in normal code. Use `std::unique_ptr`, `std::vector` and `std::string`. SQLite handles are wrapped in classes that close them in the destructor (RAII).
- **Errors:** errors are reported with exceptions derived from `std::runtime_error` (for example `ValidationError`, `DatabaseError`). Each menu catches them and shows a message, so the program never crashes on bad input (NFR-07).
- **Input:** every value typed by a user is checked before it reaches a service: numbers, dates, mark ranges and empty reasons.
- **Time:** no code reads the clock directly. It asks `IClock`, so tests can fix the time.
- **const:** methods that do not change an object are marked `const`. Parameters that are not changed are passed by `const&`.

## 5. Comments
- Each class has a one-line comment saying what it does.
- A comment names the rule it implements, for example `// BR-01: 10% of max mark per day late`, so the RTM can point to it.

## 6. Git
- Small commits with a clear message in the imperative, for example "Add SubmissionStatus enum".
- `main` always builds. New work goes on a feature branch and is merged by pull request (the full branching strategy comes in Phase 6).
