# Requirements Traceability Matrix (updated for Phase 8, CR1)

Each requirement is traced from its use case and design element (Week 3, Table 3) to its implementation and tests. Source paths are under `src/`; test identifiers are from `tests/`. Phase 5 evidence: 214/214 CTest tests passed and `src/services` line coverage was 580/594 (97.6%). Phase 8 (CR1): 218/218 CTest tests passed; coverage was not measured again after CR1.

## Functional requirements

| FR | Use case | Design element (Week 3) | Source files (Phase 4) | Test (Phase 5) |
|---|---|---|---|---|
| FR-01 | UC-01 | AuthService, IPasswordHasher, Argon2Hasher | `services/AuthService.cpp` (`login`), `services/Argon2Hasher.cpp`, `ui/LoginMenu.cpp` | `AuthServiceTest.CorrectPasswordReturnsTheUserWithTheRightRole`, `System.ST01_login_roles` |
| FR-02 | UC-01 | Role menus, `User::role()`, service checks | `ui/LoginMenu.cpp` (`menuFor`), `ui/*Menu.cpp`; checks in `AssignmentService::requireLecturerOf`, `MarkingService::requireAssignmentTaughtBy`, `ModerationService::requireAssignmentInDepartment`, `AuditService::entriesFor` | `AuditServiceTest.LecturersAndStudentsCannotSeeTheTrail`, `System.ST01_login_roles` |
| FR-03 | UC-02 | AdminService, UserFactory | `services/AdminService.cpp` (`createUser`, `updateUserName`, `deactivateUser`), `domain/UserFactory.cpp`, `domain/User.h`, `data/SqliteUserRepository.cpp`, `ui/AdminMenu.cpp` (`AdminUsersMenu`) | `AdminServiceTest.CreateUserStoresAHashNotThePassword`; `System.ST03_manage_users` |
| FR-04 | UC-03 | AdminService, ICourseRepository, Department, Course | `services/AdminService.cpp` (`createDepartment`, `renameDepartment`, `replaceHod`, `createCourse`, `updateCourse`), `data/SqliteCourseRepository.cpp`, `data/Transaction.h`, `ui/AdminMenu.cpp` (`AdminCoursesMenu`) | `AdminServiceTest.CreateDepartmentMakesTheLecturerItsHod`; `System.ST04_departments_courses` |
| FR-05 | UC-04 | `AdminService::assignLecturer()` | `services/AdminService.cpp` (`assignLecturer`), `data/SqliteCourseRepository.cpp` (`addLecturer`) | `AdminServiceTest.AssignLecturerLetsThemTeach`; `System.ST05_teaching_enrolment` |
| FR-06 | UC-05 | `AdminService::enrol()` | `services/AdminService.cpp` (`enrol`, `unenrol`), `data/SqliteCourseRepository.cpp` (`addEnrolment`, `removeEnrolment`) | `AdminServiceTest.EnrolAndUnenrol`; `System.ST05_teaching_enrolment` |
| FR-07 | UC-06 | AssignmentService, Assignment | `services/AssignmentService.cpp` (`createAssignment`), `domain/Assignment.h`, `data/SqliteAssignmentRepository.cpp`, `ui/LecturerMenu.cpp` | `AssignmentServiceTest.LecturerOfTheCourseCanCreate`; `System.ST06_create_assignment` |
| FR-08 | UC-07 | `AssignmentService::grantExtension()`, Extension, IPenaltyPolicy | `services/AssignmentService.cpp` (`grantExtension`), `domain/Extension.h`, `domain/Submission.cpp` (`setLateness`) | `AssignmentServiceTest.ExactlyFourteenDaysIsAllowed`; `System.ST07_extension` |
| FR-09 | UC-09 | `SubmissionService::listForStudent()` | `services/SubmissionService.cpp` (`listForStudent`), `domain/Assignment.h` (`effectiveDeadline`), `ui/StudentMenu.cpp` | `SubmissionServiceTest.ListShowsTheExtendedDeadline`; `System.ST08_submit_resubmit` |
| FR-10 | UC-10 | SubmissionService, IPenaltyPolicy, IClock, FileStore | `services/SubmissionService.cpp` (`submit`, `firstSubmission`), `services/FileStore.cpp`, `data/SqliteSubmissionRepository.cpp` | `SubmissionServiceTest.LateFirstSubmissionIsAcceptedAsLate`; `System.ST08_submit_resubmit` |
| FR-11 | UC-10 | `Submission::canResubmit()` | `services/SubmissionService.cpp` (`resubmission`), `domain/Submission.cpp` (`canResubmit`) | `SubmissionServiceTest.ResubmitBeforeDeadlineAddsVersion2AndKeepsVersion1`; `System.ST08_submit_resubmit` |
| FR-12 | UC-08 | MarkingService | `services/MarkingService.cpp` (`mark`, `latestFile`), `domain/Submission.cpp` (`recordMark`) | `MarkingServiceTest.FeedbackIsRequired`, `System.ST09_mark_amend` |
| FR-13 | UC-08 | IPenaltyPolicy, TenPercentPerDayPolicy | `services/IPenaltyPolicy.h`, `services/TenPercentPerDayPolicy.cpp` | `PenaltyPolicyTest.OneDayLateLosesTenPercentOfMax`, `MarkingServiceTest.PenaltyIsAppliedAutomatically` |
| FR-14 | UC-12 | ModerationService | `services/ModerationService.cpp` (`listForModeration`, `approveAll`, `adjust`, `latestFile`), `domain/Submission.cpp` (`moderate`), `ui/HodMenu.cpp` | `ModerationServiceTest.ApproveAllKeepsTheMarks`, `System.ST10_moderate_release` |
| FR-15 | UC-13 | `ModerationService::release()` | `services/ModerationService.cpp` (`release`), `data/SqliteSubmissionRepository.cpp` (`countNotModerated`), `domain/Submission.cpp` (`release`) | `ModerationServiceTest.ReleaseIsRefusedUntilEverythingIsModerated`, `System.ST10_moderate_release` |
| FR-16 | UC-11 | `SubmissionService::releasedResults()` | Implemented inside `SubmissionService::listForStudent` (`statusForStudent`; feedback only when Released) | `SubmissionServiceTest.ReleasedMarkAndFeedbackAreShown`, `System.ST10_moderate_release` |
| FR-17 | UC-08, UC-12, UC-14 | AuditService, IAuditRepository | `services/AuditService.cpp`, `data/SqliteAuditRepository.cpp`, `domain/AuditEntry.h`, `ui/Menu.cpp` (`showAudit`) | `AuditServiceTest.RecordStoresWhoWhenOldNewAndWhy`, `SqliteServiceTest.FailedAuditInsertUndoesTheMark`; `System.ST10_moderate_release` |
| FR-18 | UC-08 | `MarkingService::amend()` | `services/MarkingService.cpp` (`amend`) | `MarkingServiceTest.AmendChangesTheMarkAndIsAudited`, `System.ST09_mark_amend` |
| FR-19 | UC-15 | `AuthService::changePassword()` | `services/AuthService.cpp` (`changePassword`), `ui/Menu.cpp` (`changePassword`) | `AuthServiceTest.AfterChangingOnlyTheNewPasswordWorks`, `System.ST11_change_password` |
| FR-20 | UC-16 | BackupService | `services/BackupService.cpp`, `ui/AdminMenu.cpp` (`backUp`) | `SqliteServiceTest.BackupCopiesTheDatabaseAndTheFiles`, `System.ST12_backup` |
| FR-21 | UC-13 | Deferred from v1.0 | – | – |
| FR-22 | UC-11 | Deferred from v1.0 | – | – |
| FR-23 | UC-07 | CR1 (Phase 8): class-wide deadline extension | `services/AssignmentService.cpp` (`extendClassDeadline`), `gui/LecturerWindow.cpp` | `AssignmentServiceTest.ClassWideExtensionUpdatesDeadline`, `AssignmentServiceTest.ClassWideExtensionPreservesIndividualExtension`, `SubmissionServiceTest.ClassWideExtensionRecalculatesLateSubmission`, `SubmissionServiceTest.ClassWideExtensionPreservesMarkedSubmission` |
| FR-24 | UC-06, UC-09 | Added after Phase 4: assignment brief file | `services/AssignmentService.cpp` (`attachBrief`), `services/FileStore.cpp` (`copyBrief`), `services/SubmissionService.cpp` (`briefFor`, `removeOrphanFiles`), `data/Database.cpp` (`upgradeSchema`), `data/SqliteAssignmentRepository.cpp` (`brief_path`), `ui/LecturerMenu.cpp`, `ui/StudentMenu.cpp`, `gui/LecturerWindow.cpp`, `gui/StudentWindow.cpp` | `AssignmentServiceTest.AttachBriefCopiesTheFile`, `SubmissionServiceTest.EnrolledStudentGetsTheBrief`; `System.ST06_create_assignment` |

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
| NFR-06 | `ui/Menu.cpp` (`kMaxOptions`), `ui/Console.cpp` (`confirm`), confirmations in `ui/HodMenu.cpp` and `ui/AdminMenu.cpp`; desktop: `gui/GuiCommon.cpp` (`confirm`) in `gui/HodWindow.cpp` and `gui/AdminWindow.cpp` |
| NFR-07 | `ui/Menu.cpp` (`run` catches every exception), `gui/GuiCommon.cpp` (`runAction`), `ui/Console.cpp` (re-asking reads), `data/Transaction.h`, `services/SubmissionService.cpp` (file clean-up) |
| NFR-08 | `services/MarkingService.cpp` (`save`), `services/ModerationService.cpp` (`adjust`): mark and audit in one `Transaction` |
| NFR-09 | Phase 5 (GoogleTest, `tests/`) |
| NFR-10 | `CMakeLists.txt`, `.github/workflows/build.yml` (Ubuntu 22.04 and Windows) |
| NFR-11 | `services/BackupService.cpp` (`VACUUM INTO` + copy of submitted files) |

## Desktop GUI (`lms_gui`)

The Qt program calls the same services as the console, so every row above stays the same. Only the presentation files differ:

| Console (`src/ui`) | Desktop (`src/gui`) | Requirements |
|---|---|---|
| `LoginMenu` (`menuFor`), `App` | `LoginDialog`, `FirstAdminDialog`, `GuiApp` (`windowFor`) | FR-01, FR-02 |
| `StudentMenu` | `StudentWindow` | FR-09, FR-10, FR-11, FR-16 |
| `LecturerMenu` | `LecturerWindow` | FR-07, FR-08, FR-12, FR-18 |
| `HodMenu` | `HodWindow`, `AuditPage` | FR-14, FR-15, FR-17 |
| `AdminMenu`, `AdminUsersMenu`, `AdminCoursesMenu` | `AdminWindow`, `AuditPage` | FR-03 to FR-06, FR-17, FR-20 |
| `Menu::changePassword` | `ChangePasswordDialog` | FR-19 |
| `Menu::run` | `RoleWindow`, `runAction` | NFR-06, NFR-07 |

## Phase 5 system and acceptance test evidence

The automated console scenarios are `ST01`–`ST14`, described in `docs/SYSTEM_TESTS.md`. The GUI acceptance checks are `GT-01`–`GT-16` in the same document. They supplement the exact GoogleTest cases in the Test column.

| Requirement group | System / GUI checks |
|---|---|
| FR-01–02 | ST01; GT-02 |
| FR-03 | ST03; GT-03 |
| FR-04 | ST04; GT-04 |
| FR-05–06 | ST05; GT-05 |
| FR-07, FR-24 | ST06; GT-06, GT-07 |
| FR-08 | ST07; GT-09 |
| FR-09–11 | ST08; GT-08 |
| FR-12, FR-18 | ST09; GT-10 |
| FR-13 | PenaltyPolicyTest; MarkingServiceTest; SqliteServiceTest |
| FR-14–17 | ST10; GT-11–14 |
| FR-19 | ST11; GT-16 |
| FR-20 | ST12; GT-15 |
| NFR-07 | ST14 |
| NFR-09 | gcovr: 580/594 service lines (97.6%) |

Deferred FR-21 and FR-22 are not assigned v1.0 tests. FR-23 (CR1) is covered by the four service tests above.
