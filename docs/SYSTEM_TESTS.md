# System and acceptance tests (Phase 5)

**What these tests check.** Each test checks one use case end to end, on the finished program, the way a user would do it. Unit and integration tests check the classes; these check the whole program.

**Two sets of tests:**
- **Console, automated.** These run on `lms` / `lms.exe`.
  - Each test types `tests/system/setup.txt` plus its own `STxx_*.txt` into the real program, line by line.
  - The output must then contain the lines in `STxx_*.expect`, in that order. A line starting with `!` must not appear at all.
  - `ctest` runs them on every build, and GitHub Actions runs them on Windows and Ubuntu.
  - Each test's full output is saved in `build/system-tests/<test>/output.txt`. That file is the "actual result".
- **Desktop GUI, acceptance checks.** These run on `lms_gui`.
  - They were run with Qt in "offscreen" mode by a test program that fills in the fields and presses the buttons.
  - Screenshots are in `evidence/gui_acceptance`.

**Starting data in every console test (`setup.txt`):**

| Item | What `setup.txt` creates |
|---|---|
| Users | admin [1], mpho [2] (becomes HoD of Science), thabo [3] (lecturer of CS101), lerato [4] (lecturer, not of CS101), itu [5] and neo [6] (students in CS101), sipho [7] (student, not enrolled) |
| Course | department Science [1], course CS101 [1] |
| Assignment | thabo creates Project 1 [1], due 2099-12-31 17:00, out of 100 |

**Limitation (honest).** The console program uses the real clock, so a late submission can't be made on purpose in a system test. The deadline must be in the future when the assignment is created. Lateness and the penalty (BR-01, BR-02, FR-13) are therefore proven by:
- the unit tests (`PenaltyPolicyTest`);
- the service tests with the fake clock (`MarkingServiceTest.PenaltyIsAppliedAutomatically`);
- the SQLite integration test `SqliteServiceTest.SubmitMarkModerateRelease` (1 day late: 70 becomes 60).

## Console system tests (all PASS)

| ID | Use case / requirements | What is done | Expected result | Actual |
|---|---|---|---|---|
| ST01 | UC-01; FR-01, FR-02, NFR-07 | Wrong password, unknown user, then each of the 4 roles logs in; letters and 9 typed at a menu | Same "Invalid username or password" for both failures; each role gets its own menu; bad menu input is asked again | PASS |
| ST02 | UC-01; NFR-04 | neo types 5 wrong passwords, then the right one; itu logs in | After 5 failures: "Account locked until …", even with the right password. itu is not affected | PASS |
| ST03 | UC-02; FR-03, NFR-06 | Short password, duplicate username, rename, deactivate (answer n, then y), deactivate the HoD | Each refusal shown. "n" changes nothing. HoD refused ("Replace the HoD first"). Deactivated neo can't log in | PASS |
| ST04 | UC-03; FR-04 | Make a student HoD; create Arts; rename; course in an unknown department; replace Arts' HoD; change a course | Student refused; unknown department refused; list shows both departments with HoDs; the old HoD can't log in; the new HoD gets the HoD menu | PASS |
| ST05 | UC-04, UC-05; FR-05, FR-06 | Enrol sipho; enrol a lecturer; assign a student as lecturer; assign lerato; unenrol sipho (confirm) | Wrong roles refused; sipho sees Project 1, then nothing after removal; lerato sees CS101 | PASS |
| ST06 | UC-06; FR-07, FR-24 | Impossible date (31 Feb); max mark 101; empty title; past deadline; a wrong brief path; another lecturer's course | Each refused with its message; Project 2 created with brief; **D-09**: a wrong brief path says the assignment *was* created without a brief; the student sees both assignments and the brief | PASS |
| ST07 | UC-07; FR-08, BR-03 | Extensions: 20 days, no reason, earlier than the deadline, a student not enrolled, then 5 days for itu | Four refusals with their messages; itu sees the new deadline, neo the normal one | PASS |
| ST08 | UC-09, UC-10; FR-09 to FR-11, BR-05 | Missing file, empty file, unknown assignment; submit with a quoted path; resubmit; sipho submits; lecturer opens the file | Refusals shown; "Received on time"; one submission with version 2 as the file to mark; sipho refused (not enrolled) | PASS |
| ST09 | UC-08; FR-12, FR-18, BR-06, BR-07 | lerato marks (not her course); mark 101; letters; blank feedback; mark 70; mark again; amend without and with a reason; student views and resubmits | All refusals shown; 70.0 saved, then 75.0; the student sees "Being marked" and **not** the feedback; resubmission after marking refused | PASS |
| ST10 | UC-11 to UC-14; FR-14 to FR-17, BR-08, NFR-06 | HoD: release (refused), approve (n, then y), adjust without/with a reason, release, adjust after release; students view; admin views audit | "Cannot release: 2 submission(s) are not moderated yet"; 2 approved; 2 released; students see 72.0 and 65.0 with feedback; audit has 2 first marks and 2 adjustments | PASS |
| ST11 | UC-15; FR-19 | Wrong current password; short new one; two different new ones; then a correct change | Three refusals; "Password changed"; the old password fails and the new one works | PASS |
| ST12 | UC-16; FR-20, NFR-11 | Back up to backups/first twice | First: saved (lms.db and submissions copied); second: refused, the folder exists | PASS |
| ST13 | Start-up (Week 3, Section 4.3) | A file left in `submissions/` by a "crash" before the program starts | "Removed 1 unfinished upload(s)." and normal start | PASS |
| ST14 | NFR-07 | 17 more wrong inputs across all four roles: menu numbers out of range, letters, empty username, unknown ids, negative ids, a 5,000-character title | Every one refused with a message; the program never stops ("Goodbye." at the end; no "Something went wrong") | PASS |

**NFR-07 count.** Across ST01–ST14 the program refused **64 invalid inputs** with a message, and never stopped. The requirement asks for 30. Count the `  ! ` lines in the saved outputs.

## Desktop GUI acceptance checks (all PASS)

| ID | Use case / requirements | Check | Actual (from the run) | Result |
|---|---|---|---|---|
| GT-01 | UC-01 first start | Two different passwords when creating the administrator | "The two passwords are different." then created | PASS |
| GT-02 | UC-01; FR-01, FR-02 | Wrong password; then the administrator logs in | "Invalid username or password"; administrator window opens | PASS |
| GT-03 | UC-02; FR-03 | Create 4 users; one with a 5-letter password | "Password must be at least 8 characters"; 5 users listed | PASS |
| GT-04 | UC-03; FR-04 | Department Science with HoD Mpho; course CS101 | Department table shows HoD "Mpho Moloi" by name | PASS |
| GT-05 | UC-04, UC-05; FR-05, FR-06 | Assign thabo; enrol itu and neo | "Thabo Mokoena (thabo) now teaches CS101 …" | PASS |
| GT-06 | UC-06; FR-07, FR-24 | Empty title; then Project 1 with a brief | "Title must not be empty"; "… The brief is attached."; Brief column "yes" | PASS |
| GT-07 | UC-06; D-09 | Wrong brief path | "Assignment created with number 2. The brief was not attached; use "Attach or replace brief"." | PASS |
| GT-08 | UC-09, UC-10; FR-09, FR-10 | Home page to-do list; submit a missing file; then a real one | 2 to-do rows; "File not found"; "Received on time" | PASS |
| GT-09 | UC-07; FR-08, BR-03 | Extension of 30 days, then 8 days | "An extension can be at most 14 days"; then granted | PASS |
| GT-10 | UC-08; FR-12, FR-18 | Open the submitted file; mark 70 and 60; amend without and with a reason | File opened (sub1_v1.txt); "70.0"; "A reason is required"; "62.0" | PASS |
| GT-11 | UC-11; BR-07 | Student views before release | "Being marked", feedback empty | PASS |
| GT-12 | UC-12, UC-13; FR-14, FR-15, BR-08 | Release before moderation; approve; adjust without and with a reason; release | "Cannot release: 2 submission(s) are not moderated yet"; "2 submission(s) approved"; reason required; "2 mark(s) released" | PASS |
| GT-13 | UC-14; FR-17 | HoD's audit trail | 4 rows: 2 first marks, the amendment, the adjustment | PASS |
| GT-14 | UC-11; FR-16 | Student views after release | "Released: 70.0 / 100" and "Good structure" | PASS |
| GT-15 | UC-16, UC-02; FR-20 | Backup; deactivate neo; neo logs in | "Backup saved in backups/gui1 …"; neo refused | PASS |
| GT-16 | UC-15; FR-19 | Change password in the dialog | "Password changed."; only the new password works | PASS |

## How to run them

```
cmake --build build
ctest --test-dir build -R "^System[.]" --output-on-failure
```
- If a test fails, ctest prints which expected line was missing.
- The program's whole output is in `build/system-tests/<test>/output.txt`.
- To add a test, copy any `STxx` pair of files, change them, and run `cmake -S . -B build` once.
