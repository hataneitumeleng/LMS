# CS4433 — Phase 8: Class-Wide Deadline Extensions

**Project:** Moderated Learning Management System (LMS)  
**Student:** Itumeleng Hatane  
**Date:** 2 November 2026  
**Pull request:** [#2](https://github.com/hataneitumeleng/LMS/pull/2) (merged into `main`)

## Objective

Add class-wide assignment deadline extensions without overwriting individual extensions or changing already-marked work.

## Implementation

The lecturer workflow updates the assignment's main deadline. The assignment service saves the new deadline and recalculates lateness for existing open (`Submitted` or `Late`) submissions using each student's effective deadline. Individual extensions remain in place; `Marked`, `Moderated`, and `Released` submissions are preserved.

## Testing and results

- Regression tests cover class-wide deadline updates, individual-extension preservation, lateness recalculation, and marked-submission preservation.
- Full local CTest run: **218/218 passed**, **0 failed** (157.18 seconds).
- GUI acceptance: **0 failed** after correcting GT-09 to restore the reason for the second attempt and use deadline-relative dates.
- GitHub Actions: **six checks passed** on the final pull request revision.

## Git history

- `7547519` — Add class-wide deadline extensions with regression tests.
- `e346df2` — Fix GUI acceptance extension test.
- `1cd0aea` — Merge Pull Request #2 into `main`.

## Evidence

| Evidence | Path |
|---|---|
| Full automated test output | `evidence/phase8/automated_tests.txt` |
| GUI acceptance output | `evidence/phase8/gui_acceptance_result.txt` |
| 218/218 tests screenshot | `evidence/phase8/01_automated_tests.png` |
| GUI acceptance screenshot | `evidence/phase8/02_gui_acceptance.png` |
| GitHub Actions screenshot | `evidence/phase8/03_github_actions.png` |
| Merged pull request screenshot | `evidence/phase8/04_pull_request_merged.png` |

The evidence files must be copied into the repository before these relative paths become available on GitHub.

## Conclusion

Phase 8 was implemented, regression-tested, and merged. Class-wide deadline changes recalculate open submissions while retaining individual extensions and preserving marked work. All recorded local automated tests passed, the GUI acceptance suite reported zero failures, and the final GitHub CI checks passed.
