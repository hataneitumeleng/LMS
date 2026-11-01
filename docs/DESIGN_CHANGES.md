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
- **App (shared start-up):** `System` (`src/app/System.cpp`) is now the composition root. It builds every concrete object once, and both programs start from it. `Services` moved from `src/ui` to `src/app`, because both front ends use it.
- **Desktop GUI (`src/gui`, program `lms_gui`):** a second presentation layer in Qt 6 Widgets, next to the console. See section 7.

## 5. Decisions made during coding

- **HoD:** an HoD is a promoted lecturer. `createUser` refuses the HoD role; `createDepartment` and `replaceHod` promote a lecturer in one transaction.
- **Structs:** `Assignment`, `Extension`, `Submission`, `SubmissionVersion`, `AuditEntry`, `Course` and `Department` are structs with public fields (plus a few methods), not classes with private fields.
- **Audit records:** the audit trail records the **final** mark. A first mark has no old value and the reason "First mark".
- **Resubmission:** a resubmission works out lateness again from the new version's time.
- **Extensions:** a second extension replaces the first (open issue 4 from Week 3).
- **File checks:** a submitted file must exist and not be empty. No size or type limit was in the requirements.

## 6. Known limitations of v1.0

- Console only: the password is visible while typing (hidden in the desktop GUI).
- Lists show user ids, not names. The desktop GUI shows names in the administrator's lists, but lecturers and HoDs still see student numbers.
- An HoD has only the HoD menu, so an HoD who also teaches needs a separate lecturer account.

## 7. Desktop GUI (added after Phase 4)

The brief allows "a console or simple desktop GUI". A Qt desktop program, `lms_gui`, was added as a second presentation layer. The console program `lms` is unchanged and still works.

- **No business code changed.** The windows call the same services as the menus, and the services still check every rule and every permission. This is what the strict layering in Figure 1 was for: a new presentation layer needed no change in the services, the data layer or the database.
- **Same data:** both programs use `lms.db` and the `submissions` folder, so a user can work in either.
- **Classes** (Figure 1 gets a "Desktop GUI" box beside "Console UI"):
  - `RoleWindow`, the base class of the four role windows: Account menu (change password, log out) and one tab per task. It plays the part of `Menu`.
  - `StudentWindow`, `LecturerWindow`, `HodWindow`, `AdminWindow`, and `AuditPage`, the audit tab shared by the HoD and administrator.
  - `LoginDialog`, `FirstAdminDialog` and `ChangePasswordDialog`.
  - `GuiApp`: first start, login, the role's window, and back to login after "Log out". `GuiApp::windowFor` picks the window from the user's role (FR-02), like `LoginMenu::menuFor`.
  - `GuiCommon`: `runAction` catches every error and shows it in a message box (NFR-07), like `Menu::run`; `confirm` asks before release, approve all, deactivate, replace HoD and remove a student (NFR-06).
- **Qt widgets are created with `new` and a parent widget.** Qt deletes child widgets with their parent, so this is still automatic memory management. It is the one exception to the "no `new`" rule in the coding standard.
- **Optional build:** CMake builds `lms_gui` only when Qt 6 is installed, so the console still builds without Qt. The GitHub build installs Qt on both systems.
- **Student Home page:** the student's window opens on a Home tab with a welcome, their courses, a count of work to submit, waiting for a mark and released, and a "Still to submit" list with the time left (double-click a row to open it). It uses only `SubmissionService::listForStudent`. For "now" it uses `IClock`, so `Services` now also carries the clock.
- **Opening a submitted file:** new read-only methods `MarkingService::latestFile(lecturerId, submissionId)` and `ModerationService::latestFile(hodId, submissionId)` return the latest `SubmissionVersion` (BR-05: the latest version is the one marked). They check the same permissions as marking and moderation (FR-02). The windows have an "Open submitted file" button (or double-click the row), which opens the file in its normal program. The lecturer and HoD console menus gained "Show a submitted file", which prints the file's full path, so "Change my password" moved from 7 to 8.
- **New requirement FR-24, assignment brief file (Should, added 1 Nov 2026 at the student's request):** a lecturer can attach a file (instructions, a template) when creating an assignment, or attach or replace it later; enrolled students can open it.
  - **Schema:** new nullable column `assignment.brief_path`. `Database::upgradeSchema()` adds it to a database made by an older version (`ALTER TABLE`), so existing data is kept.
  - **Services:** `AssignmentService::attachBrief` (its constructor now also takes `const FileStore&`); `SubmissionService::briefFor` (only for students enrolled in the course, FR-02); `IAssignmentRepository::allBriefPaths`, so start-up clean-up keeps brief files; `StudentAssignmentRow::hasBrief`.
  - **Files:** `FileStore::copyBrief` saves `brief<id>_<n>.<ext>` in the submissions folder, so backups include briefs. A new brief gets a new name, and the old file is deleted only after the database is updated. A failed save deletes the new copy, as in Figure 6.
  - **Console:** creating an assignment asks for an optional brief file; the lecturer menu has "Attach or replace an assignment brief" (8; "Change my password" is now 9, still within the 9-option limit); students and lecturers see the brief's full path in their lists.
- **Two console limitations are fixed in the GUI:** the password is hidden while typing, and the administrator picks users, departments and courses by name from lists.
