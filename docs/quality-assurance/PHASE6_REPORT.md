# Phase 6 - Quality Assurance and Configuration Management Report

**Project:** Moderated Learning Management System (LMS)  
**Course:** CS4433 Software Engineering  
**Repository:** https://github.com/hataneitumeleng/LMS  
**Branch:** `feature/phase6-quality-assurance`

## 1. Introduction

Phase 6 focused on assessing the quality of the LMS implementation and verifying the project's configuration-management practices. The activities included static code analysis, code metrics, self code review, version control and continuous integration.

## 2. Static Code Analysis

Cppcheck 2.21.0 was used to analyse the C++ implementation.

A total of 44 source files were analysed. The initial scan identified three warnings.

| File | Finding | Action |
|---|---|---|
| `SubmissionService.cpp` | Ineffective substring assignment | Corrected using `resize()` |
| `ModerationService.cpp` | Suggested use of `std::copy_if` | Reviewed and retained existing loop |
| `AdminWindow.cpp` | Suggested use of `std::find_if` | Reviewed and retained existing loop |

After the correction, Cppcheck was executed again.

**Final result:** Two style warnings remained, with no errors reported.

The remaining warnings were accepted because the existing loops were considered clear and did not represent confirmed functional defects.

## 3. Code Metrics

The LMS implementation contains 105 C++ source and header files under `src`, with 5,240 lines reported by the PowerShell line-count command.

Lizard 1.24.3 was used to measure cyclomatic complexity.

| Function | CCN |
|---|---:|
| `SubmissionService::submit()` | 6 |
| `SubmissionService::resubmission()` | 3 |
| `SubmissionService::statusForStudent()` | 8 |
| `ModerationService::approveAll()` | 3 |
| `ModerationService::adjust()` | 5 |
| `ModerationService::release()` | 5 |

The selected functions were below the configured complexity warning threshold of 15. The Lizard report did not flag any threshold exceedances.

## 4. Self Code Review

A self code review was carried out using a checklist covering requirements traceability, architecture, security, input validation, business rules, audit logging, maintainability and testing.

The review resulted in one code-quality correction:

**File:** `src/services/SubmissionService.cpp`  
**Function:** `statusForStudent()`

The unnecessary substring assignment was replaced with `resize()`.

After rebuilding the application, all 214 automated tests passed.

The other two Cppcheck suggestions were reviewed and accepted without changing the implementation.

The complete review record is available in `SELF_CODE_REVIEW.md`.

## 5. Configuration Management

Git and GitHub are used for version control.

The repository uses `main` as the integration branch. Phase 6 work was carried out on a separate feature branch:

`feature/phase6-quality-assurance`

The existing Git tags `phase4-submitted` and `gui-8d` identify earlier project milestones.

The Phase 6 workflow uses a feature branch, automated CI verification and a planned pull request before merging into `main`.

## 6. Continuous Integration

The GitHub Actions workflow is defined in `.github/workflows/build.yml`.

It is configured to run automatically on pushes and pull requests.

GitHub Actions build #13 completed successfully on the Phase 6 feature branch at commit `80db1c7`.

| CI job | Result |
|---|---|
| Ubuntu | Passed |
| Windows | Passed |
| Coverage | Passed |

The overall workflow completed in 4 minutes and 28 seconds.

This run verifies the previously committed project state. The Phase 6 documentation and code-quality correction will be verified through CI after they are pushed.

## 7. Supporting Evidence

The following records are maintained under `docs/quality-assurance/`:

- `cppcheck-report.txt` - initial static analysis results.
- `cppcheck-final.txt` - final static analysis results.
- `complexity-report.txt` - numerical code-complexity measurements.
- `complexity-check.txt` - supplementary Cppcheck analysis.
- `SELF_CODE_REVIEW.md` - review checklist, findings and actions.
- `CONFIGURATION_MANAGEMENT.md` - branching, version control and CI practices.

Additional evidence includes the Git diff for `SubmissionService.cpp`, the CTest results and GitHub Actions build #13.

## 8. Conclusion

Phase 6 demonstrated that the LMS implementation was reviewed using static analysis and numerical code metrics. One code-quality warning was corrected, two style suggestions were assessed and accepted, and all 214 regression tests passed after the correction.

GitHub Actions successfully completed the Ubuntu, Windows and coverage jobs. The remaining configuration-management activity is to commit the Phase 6 work, verify the resulting CI run and complete the pull request.