# Requirements Traceability Matrix (Phase 4: source column)

Each requirement is traced from its use case and design element (Week 3, Table 3) to the source files that implement it. All paths are under `src/`. The **Test** column is filled in during Phase 5.

## Functional requirements

| FR | Use case | Design element (Week 3) | Source files (Phase 4) | Test (Phase 5) |
|---|---|---|---|---|
| FR-01 | UC-01 | AuthService, IPasswordHasher, Argon2Hasher | `services/AuthService.cpp` (`login`), `services/Argon2Hasher.cpp`, `ui/LoginMenu.cpp` | |
| FR-02 | UC-01 | Role menus, `User::role()`, service checks | `ui/LoginMenu.cpp` (`menuFor`), `ui/*Menu.cpp`; checks in `AssignmentService::requireLecturerOf`, `MarkingService::requireAssignmentTaughtBy`, `ModerationService::requireAssignmentInDepartment`, `AuditService::entriesFor` | |
| FR-03 | UC-02 | AdminService, UserFactory | `services/AdminService.cpp` (`createUser`, `updateUserName`, `deactivateUser`), `domain/UserFactory.cpp`, `domain/User.h`, `data/SqliteUserRepository.cpp`, `ui/AdminMenu.cpp` (`AdminUsersMenu`) | |
| FR-04 | UC-03 | AdminService, ICourseRepository, Department, Course | `services/AdminService.cpp` (`createDepartment`, `renameDepartment`, `replaceHod`, `createCourse`, `updateCourse`), `data/SqliteCourseRepository.cpp`, `data/Transaction.h`, `ui/AdminMenu.cpp` (`AdminCoursesMenu`) | |
| FR-05 | UC-04 | `AdminService::assignLecturer()` | `services/AdminService.cpp` (`assignLecturer`), `data/SqliteCourseRepository.cpp` (`addLecturer`) | |
| FR-06 | UC-05 | `AdminService::enrol()` | `services/AdminService.cpp` (`enrol`, `unenrol`), `data/SqliteCourseRepository.cpp` (`addEnrolment`, `removeEnrolment`) | |
| FR-07 | UC-06 | AssignmentService, Assignment | `services/AssignmentService.cpp` (`createAssignment`), `domain/Assignment.h`, `data/SqliteAssignmentRepository.cpp`, `ui/LecturerMenu.cpp` | |
| FR-08 | UC-07 | `AssignmentService::grantExtension()`, Extension, IPenaltyPolicy | `services/AssignmentService.cpp` (`grantExtension`), `domain/Extension.h`, `domain/Submission.cpp` (`setLateness`) | |
| FR-09 | UC-09 | `SubmissionService::listForStudent()` | `services/SubmissionService.cpp` (`listForStudent`), `domain/Assignment.h` (`effectiveDeadline`), `ui/StudentMenu.cpp` | |
| FR-10 | UC-10 | SubmissionService, IPenaltyPolicy, IClock, FileStore | `services/SubmissionService.cpp` (`submit`, `firstSubmission`), `services/FileStore.cpp`, `data/SqliteSubmissionRepository.cpp` | |
| FR-11 | UC-10 | `Submission::canResubmit()` | `services/SubmissionService.cpp` (`resubmission`), `domain/Submission.cpp` (`canResubmit`) | |
| FR-12 | UC-08 | MarkingService | `services/MarkingService.cpp` (`mark`), `domain/Submission.cpp` (`recordMark`) | |
| FR-13 | UC-08 | IPenaltyPolicy, TenPercentPerDayPolicy | `services/IPenaltyPolicy.h`, `services/TenPercentPerDayPolicy.cpp` | |
| FR-14 | UC-12 | ModerationService | `services/ModerationService.cpp` (`listForModeration`, `approveAll`, `adjust`), `domain/Submission.cpp` (`moderate`), `ui/HodMenu.cpp` | |
| FR-15 | UC-13 | `ModerationService::release()` | `services/ModerationService.cpp` (`release`), `data/SqliteSubmissionRepository.cpp` (`countNotModerated`), `domain/Submission.cpp` (`release`) | |
| FR-16 | UC-11 | `SubmissionService::releasedResults()` | Implemented inside `SubmissionService::listForStudent` (`statusForStudent`; feedback only when Released) | |
| FR-17 | UC-08, UC-12, UC-14 | AuditService, IAuditRepository | `services/AuditService.cpp`, `data/SqliteAuditRepository.cpp`, `domain/AuditEntry.h`, `ui/Menu.cpp` (`showAudit`) | |
| FR-18 | UC-08 | `MarkingService::amend()` | `services/MarkingService.cpp` (`amend`) | |
| FR-19 | UC-15 | `AuthService::changePassword()` | `services/AuthService.cpp` (`changePassword`), `ui/Menu.cpp` (`changePassword`) | |
| FR-20 | UC-16 | BackupService | `services/BackupService.cpp`, `ui/AdminMenu.cpp` (`backUp`) | |
| FR-21 | UC-13 | Deferred from v1.0 | – | – |
| FR-22 | UC-11 | Deferred from v1.0 | – | – |
| FR-23 | – | Won't (CR1 in Phase 8) | – | – |

## Business rules

| BR | Source files |
|---|---|
| BR-01 | `services/TenPercentPerDayPolicy.cpp` (`finalMark`) |
| BR-02 | `services/TenPercentPerDayPolicy.cpp` (`daysLate`) |
| BR-03 | `services/AssignmentService.cpp` (`grantExtension`), `domain/Assignment.h` (`effectiveDeadline`), `sql/schema.sql` (extension primary key, reason CHECK) |
| BR-04 | `services/AssignmentService.cpp` (`grantExtension`), `domain/Submission.cpp` (`setLateness`) |
| BR-05 | `services/SubmissionService.cpp` (`resubmission`), `data/SqliteSubmissionRepository.cpp` (`latestVersion`) |
| BR-06 | `services/SubmissionService.cpp` (`submit`), `domain/Submission.cpp` (`canResubmit`) |
| BR-07 | `services/SubmissionService.cpp` (`statusForStudent`) |
| BR-08 | `services/ModerationService.cpp` (`adjust`, `requireAssignmentInDepartment`) |

## Non-functional requirements

| NFR | Source files / evidence |
|---|---|
| NFR-01, NFR-02 | `tools/benchmark.cpp` (R12 timing test); index `idx_submission_status` in `sql/schema.sql` |
| NFR-03 | `services/Argon2Hasher.cpp` (libsodium Argon2id) |
| NFR-04 | `domain/User.cpp` (`recordFailedLogin`, `isLocked`), `services/AuthService.cpp` (`checkPasswordRules`) |
| NFR-05 | No network code; SQLite file database (`data/Database.cpp`) |
| NFR-06 | `ui/Menu.cpp` (`kMaxOptions`), `ui/Console.cpp` (`confirm`), confirmations in `ui/HodMenu.cpp` and `ui/AdminMenu.cpp` |
| NFR-07 | `ui/Menu.cpp` (`run` catches every exception), `ui/Console.cpp` (re-asking reads), `data/Transaction.h`, `services/SubmissionService.cpp` (file clean-up) |
| NFR-08 | `services/MarkingService.cpp` (`save`), `services/ModerationService.cpp` (`adjust`): mark and audit in one `Transaction` |
| NFR-09 | Phase 5 (GoogleTest, `tests/`) |
| NFR-10 | `CMakeLists.txt`, `.github/workflows/build.yml` (Ubuntu 22.04 and Windows) |
| NFR-11 | `services/BackupService.cpp` (`VACUUM INTO` + copy of submitted files) |
