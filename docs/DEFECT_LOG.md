# Defect log

Every problem found from Phase 4 to Phase 5, how it was found, and what was done.

**Severity:**
- **High:** wrong data or the program can't be built or run.
- **Medium:** a feature is missing or misleads the user.
- **Low:** cosmetic, or the user can easily work around it.

| ID | Found (date, how) | Description | Severity | Cause | Fix | Status | Regression test |
|---|---|---|---|---|---|---|---|
| D-01 | 23 Oct, step 1 build test on Windows | The compiler crashed with no message (`cc1plus` exit 0xC0000139) | High | An old MinGW folder and SWI-Prolog on the Windows PATH supplied a wrong `libgmp-10.dll` | `C:\msys64\ucrt64\bin` moved to the top of the System PATH; build guide updated | Closed | CI builds on a clean Windows machine on every push |
| D-02 | 23 Oct, step 1 build test | CMake could not find the SQLite target | High | CMake 4 renamed `SQLite::SQLite3` to `SQLite3::SQLite3` | `CMakeLists.txt` uses whichever name exists | Closed | CI on Ubuntu (old CMake) and Windows (new CMake) |
| D-03 | 24 Oct, step 7 input check | Impossible dates such as 2027-02-31 were accepted and quietly became 3 March | Medium | `mktime` "normalises" invalid dates | `timeFromText` converts back and compares; extra text is refused | Closed | System test ST06 (31 Feb refused) |
| D-04 | 24 Oct, Phase 4 repository review | `submissions/` and `backups/` could be committed to git with students' files | Medium (privacy) | Missing `.gitignore` entries | Added to `.gitignore` (commit 5a322a6) | Closed | `git status` stays clean after the system tests |
| D-05 | 1 Nov, GUI test run (step 8) | The administrator's course table showed department **numbers**, not names | Low | The table filled the column from `Course::departmentId` | Look up the department name | Closed | GUI check GT-04 |
| D-06 | 1 Nov, GUI design review (step 8) | The program could quit when the login dialog closed between two windows | High | Qt ends `exec()` when the last window closes | `setQuitOnLastWindowClosed(false)` plus a `QEventLoop` per window | Closed | GUI login/logout loop test (step 8) |
| D-07 | 1 Nov, **user acceptance** (the user, as a lecturer) | A lecturer could see that a student submitted but could not **open** the file | Medium | Missing feature: no way to reach the stored file | `MarkingService::latestFile` / `ModerationService::latestFile` (permission checked), "Open submitted file" button, console option 7 (step 8c) | Closed | GUI check GT-10; system test ST08; `MarkingServiceTest.LatestFileIsTheNewestVersion` |
| D-08 | 1 Nov, **user acceptance** (the user, as a student) | After login a student went straight to the assignment list, with no overview | Low | Design only listed assignments | Student Home tab: courses, counts, "Still to submit" (step 8b) | Closed | GUI check GT-08 |
| D-09 | 1 Nov, **system test ST06** (Phase 5) | Console: when the brief path is wrong, the error appeared after "Assignment created", so it looked as if nothing was saved. A lecturer could create the assignment twice | Medium | `LecturerMenu::createAssignment` let the brief error go to the general error handler | The message now says the assignment **was** created without a brief and to use option 8 (as the GUI already did) | Closed | System test ST06; GUI check GT-07 |
| D-10 | 1 Nov, step 2 test run (test code) | Two test suites could write to the same temporary folder if run in parallel (`ctest -j`) | Low (tests only) | The folder name used only the test name, not the suite | Folder named `<suite>_<test>` | Closed | `ctest -j8` passes |

## Summary

- **Found:** 10 defects; **closed:** 10; **open:** 0.
- **Where they were found:**
  - **Building and CI:** 2 (D-01, D-02).
  - **Developer testing and review:** 5 (D-03, D-04, D-05, D-06, D-10).
  - **User acceptance:** 2 (D-07, D-08). The user tried the program and reported them.
  - **System testing:** 1 (D-09).
- **Unit, service and integration tests:** 200 tests, and none found a defect in the program code. That shows the rules were right. The deliberate-bug checks in steps 1 to 4 (mutation testing) show the tests **would** have caught mistakes in those rules.

## Known limitations (not defects; kept for v1.1)

- **Console:** the password is visible while typing (hidden in the GUI).
- **Console lists** show user numbers, not names.
- **Real clock:** the program uses the computer's clock, so a wrong PC clock would mark work late (risk R10, accepted for v1.0 in the risk register).
