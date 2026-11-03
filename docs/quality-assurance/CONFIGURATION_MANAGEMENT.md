# Phase 6 - Configuration Management Record

**Project:** Moderated Learning Management System (LMS)  
**Course:** CS4433 Software Engineering  
**Repository:** https://github.com/hataneitumeleng/LMS

## 1. Version Control

Git is used to track source code, documentation and configuration changes. GitHub hosts the remote repository.

The project history contains commits for the implementation, automated tests, GUI development and requirements traceability.

The latest verified commit before the Phase 6 changes is `80db1c7`, titled "Complete Phase 5 testing and requirements traceability".

## 2. Branching Strategy

The repository uses `main` as its primary integration branch.

For Phase 6, a separate branch named `feature/phase6-quality-assurance` was created from `main` and pushed to GitHub.

The branching procedure for Phase 6 is:

1. Create a feature branch from `main`.
2. Make and verify the required changes on the feature branch.
3. Commit and push the changes to GitHub.
4. Confirm that the CI workflow passes.
5. Open a pull request to merge the feature branch into `main`.
6. Review the changes and merge after successful checks.

This workflow is being introduced for Phase 6; earlier project work was primarily committed directly to `main`.

## 3. Version Tags

Two existing Git tags were verified:

| Tag | Purpose |
|---|---|
| `phase4-submitted` | Identifies the Phase 4 submission state |
| `gui-8d` | Identifies a GUI development milestone |

Release tags were added later: v1.0 (Phase 7 release) and v1.1 (Phase 8, change request CR1).

## 4. Continuous Integration

The GitHub Actions workflow is defined in `.github/workflows/build.yml`.

It is configured to run automatically on pushes and pull requests.

The workflow includes:

- Ubuntu 22.04 build and automated tests.
- Windows build and automated tests.
- Code coverage measurement.
- Application startup and performance checks.
- Uploading test evidence where configured.

## 5. CI Verification Evidence

GitHub Actions build #13 was triggered by a push to `feature/phase6-quality-assurance` at commit `80db1c7`.

| Job | Result | Duration |
|---|---|---|
| Ubuntu | Passed | 1 min 44 sec |
| Windows | Passed | 4 min 24 sec |
| Coverage | Passed | 1 min 38 sec |

Overall workflow result: **Success**.

Total workflow duration: **4 min 28 sec**.

## 6. Configuration Items

The main configuration items are:

| Item | Location |
|---|---|
| C++ application source | `src/` |
| Automated tests | `tests/` |
| Build configuration | `CMakeLists.txt` |
| CI workflow | `.github/workflows/build.yml` |
| Requirements traceability | `docs/RTM.md` |
| Quality-assurance records | `docs/quality-assurance/` |

Build-generated files and temporary test outputs should remain excluded from version control through `.gitignore`.

## 7. Change Control

Changes are tracked using Git commits. The Phase 6 feature branch separates quality-assurance work from the main integration branch.

The code-quality correction in `SubmissionService.cpp` was verified by rebuilding the LMS and running the regression test suite. All 214 automated tests passed.

The Phase 6 changes will be submitted through a pull request after the required documentation and verification are complete.

## 8. Conclusion

The LMS uses Git and GitHub for version control and GitHub Actions for continuous integration. Successful Ubuntu, Windows and coverage jobs provide evidence that the automated pipeline is operational.

The Phase 6 feature branch establishes a documented workflow for reviewing and merging changes without modifying the existing Git history.