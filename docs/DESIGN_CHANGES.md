# Changes from the Week 3 design (Phase 4)

This file lists every difference between the Week 3 class diagrams (Figures 2 and 3, Figures 6 and 7) and the code, so the design and the code can be checked against each other (risk R13). None of these adds or removes a requirement.

## 1. Changed signatures

| Design (Week 3) | Code | Why |
|---|---|---|
| `IUserRepository::findActiveHod()` → `optional<User>` | returns `unique_ptr<User>` | C++ can't hold an abstract class in `optional` |
| `ISubmissionRepository::save()` → `void` | returns the new id | the file name and the version row need the id |
| `BackupService::backup(path)`: one file | `backup(folder)`: `lms.db` + `submissions/` | the database only stores file paths, so the files must be copied too |
| `MarkingService::mark/amend` → `void` | return the final mark | the lecturer sees the penalty result at once |
| `ModerationService::approveAll/release` → `void` | return a count | the menu says how many were approved or released |
| `SubmissionService::releasedResults()` (FR-16) | inside `listForStudent()` | one screen shows deadlines, status and released marks |
| Figure 6: copy the file, then save the row | save the row, then copy the file | the file name contains the submission id; both are still in one transaction with file clean-up |

## 2. Extra constructor parameters (dependency injection)

| Service | Extra parameters | Why |
|---|---|---|
| AdminService | `Database&`, `ICourseRepository&`, `IPasswordHasher&` | transactions, departments and courses, hashing new passwords |
| AssignmentService | `Database&`, `ICourseRepository&`, `IUserRepository&`, `IClock&` | transaction, lecturer and enrolment checks, "now" |
| SubmissionService | `Database&`, `ICourseRepository&` | transaction, enrolment check |
| MarkingService | `Database&`, `IAssignmentRepository&`, `ICourseRepository&` | transaction, max mark, lecturer check |
| ModerationService | `Database&`, `IAssignmentRepository&`, `ICourseRepository&`, `IUserRepository&`, `IClock&` | transactions, department check, `moderated_at` |
| AuditService | `IClock&` | `changed_at` |

## 3. Added methods

- **Repositories:**
  - `IUserRepository`: `findById`, `findAll`;
  - `ICourseRepository`: `findDepartment`, `findCourse`, `allDepartments`, `allCourses`, `isLecturerOf`, `isEnrolled`, `removeEnrolment`, `coursesForLecturer`;
  - `IAssignmentRepository`: `forCourse`;
  - `ISubmissionRepository`: `findById`, `latestVersion`, `versions`, `allFilePaths`.
- **Services:**
  - `AuthService::checkPasswordRules`;
  - `AdminService`: `updateUserName`, `renameDepartment`, `updateCourse`, `unenrol`, `listUsers`, `listDepartments`, `listCourses`;
  - `AssignmentService`: `coursesTaughtBy`, `assignmentsFor`;
  - `MarkingService::listForMarking`;
  - `ModerationService::assignmentsInDepartment`;
  - `SubmissionService`: `listForStudent`, `removeOrphanFiles`;
  - `AuditService::entriesFor`.
- **Domain:** `Submission`: `isOpen`, `setLateness`, `recordMark`, `moderate`, `release`.

The list methods exist so the menus never use a repository (strict layering).

## 4. Added classes

- **Data:** `Transaction` (RAII guard for begin/commit/rollback), `Statement`, `DatabaseError`.
- **Domain:** `ValidationError`, `AuthError` (`Errors.h`) and the `TimeFormat` helpers.
- **UI:**
  - `Menu` (abstract base, Template Method), `Console`, `App` (composition root) and `Services`;
  - `AdminUsersMenu` and `AdminCoursesMenu`, which keep each menu to 9 options or fewer (NFR-06).
- **Tool:** `tools/benchmark.cpp` (R12).

## 5. Decisions made during coding

- **HoD:** an HoD is a promoted lecturer. `createUser` refuses the HoD role; `createDepartment` and `replaceHod` promote a lecturer in one transaction.
- **Structs:** `Assignment`, `Extension`, `Submission`, `SubmissionVersion`, `AuditEntry`, `Course` and `Department` are structs with public fields (plus a few methods), not classes with private fields.
- **Audit records:** the audit trail records the **final** mark. A first mark has no old value and the reason "First mark".
- **Resubmission:** a resubmission works out lateness again from the new version's time.
- **Extensions:** a second extension replaces the first (open issue 4 from Week 3).
- **File checks:** a submitted file must exist and not be empty. No size or type limit was in the requirements.

## 6. Known limitations of v1.0

- The password is visible while typing.
- Lists show user ids, not names.
- An HoD has only the HoD menu, so an HoD who also teaches needs a separate lecturer account.
