# Phase 6 — Self Code Review Record

**Project:** Moderated Learning Management System (LMS)  
**Course:** CS4433 Software Engineering  
**Review type:** Individual self code review  
**Branch:** `feature/phase6-quality-assurance`

## 1. Review Objective

The purpose of this review is to assess the maintainability, security, correctness and testability of the LMS implementation. The review considers the existing architecture, business rules, automated test evidence and static analysis findings.

## 2. Review Checklist

| Review criterion | Evidence or observation |
|---|---|
| Requirements traceability | The RTM links requirements to implementation files and tests. |
| Architecture | The implementation separates presentation, services, domain logic and data access. |
| Role-based access | Permission checks are implemented in the service layer. |
| Input validation | Validation and exception handling are used for invalid operations. |
| Password security | Passwords are stored using salted password hashes. |
| Database access | SQLite operations are handled through repositories using parameterised queries. |
| Business rules | Late penalties, extensions, resubmissions and moderation are covered by automated tests. |
| Audit logging | Mark changes are recorded through the audit mechanism. |
| Code maintainability | Cppcheck and Lizard were used to identify potential quality concerns. |
| Regression testing | All 214 automated tests passed after the code correction. |

## 3. Static Analysis Findings and Decisions

Cppcheck 2.21.0 analysed 44 C++ source files and reported three warnings.

| File | Finding | Decision | Result |
|---|---|---|---|
| `SubmissionService.cpp` | Ineffective `substr()` assignment | Replace with `resize()` | Corrected; regression tests passed |
| `ModerationService.cpp` | Suggested `std::copy_if` | Retain the existing readable filtering loop | Reviewed and accepted |
| `AdminWindow.cpp` | Suggested `std::find_if` | Retain the existing readable search loop | Reviewed and accepted |

The two retained findings are style recommendations rather than confirmed functional defects. The existing implementations were considered clear and suitable for the project.

## 4. Code Change Made

**File:** `src/services/SubmissionService.cpp`  
**Function:** `SubmissionService::statusForStudent()`

Original statement:

`mark = mark.substr(0, mark.find('.') + 2);`

Revised statement:

`mark.resize(mark.find('.') + 2);`

The revision avoids unnecessary substring assignment while preserving the intended one-decimal-place display of released marks.

## 5. Verification

After the correction, the LMS was rebuilt and tested using CMake and CTest.

- Automated tests executed: 214
- Tests passed: 214
- Tests failed: 0
- Execution time: 227.26 seconds

The change did not cause any failures in the existing regression test suite.

## 6. Code Metrics

The `src` directory contains 105 C++ source and header files, with 5,240 lines reported by the PowerShell line-count command.

Lizard 1.24.3 was used to measure cyclomatic complexity.

| Function | Cyclomatic complexity |
|---|---:|
| `SubmissionService::submit()` | 6 |
| `SubmissionService::resubmission()` | 3 |
| `SubmissionService::statusForStudent()` | 8 |
| `ModerationService::approveAll()` | 3 |
| `ModerationService::adjust()` | 5 |
| `ModerationService::release()` | 5 |

The selected functions remained below the configured Lizard complexity warning threshold of 15.

## 7. Review Conclusion

The review identified one code-quality issue that was corrected and two style suggestions that were reviewed and accepted without modification. Regression testing confirmed that the application continued to pass all 214 automated tests.

No further C++ changes were considered necessary based on these findings.

**Supporting evidence:**
- `cppcheck-report.txt`
- `complexity-report.txt`
- `complexity-check.txt`
- Git diff for `SubmissionService.cpp`
- CTest regression test results
