# Phase 7 — Deployment Verification

**Project:** Moderated Learning Management System (LMS)  
**Course:** CS4433 Software Engineering  
**Platform:** Windows  
**Verification date:** 2 November 2026

## 1. Objective

The purpose of this verification was to confirm that the LMS could be packaged, extracted and launched successfully on Windows without running the application directly from the development build directory.

## 2. Deployment Package

The Windows distribution package was created as `dist/Moderated_LMS_Windows.zip`.

The package contains the LMS console and GUI executables, Qt runtime libraries, required DLLs and Qt plugin directories.

The distribution excludes the development database, uploaded submissions and backup data.

## 3. Verification Results

| Test | Expected result | Actual result | Status |
|---|---|---|---|
| Windows packaging | ZIP archive created | Archive created successfully | Pass |
| Package inspection | No user database or submission data | No excluded data found in ZIP | Pass |
| ZIP extraction | Files extracted without errors | Extraction completed successfully | Pass |
| Application launch | GUI opens without missing-DLL errors | Login window displayed | Pass |
| Clean installation | First-time setup displayed with a fresh working directory | First start window displayed | Pass |
| Database initialization | SQLite database created | `lms.db` created (98,304 bytes) | Pass |
| Submission storage | Submissions directory created | Directory created successfully | Pass |
| Administrator setup | New administrator can be created | Administrator dashboard displayed during earlier package test | Pass |

## 4. Deployment Environment

The application was verified on the Windows development computer using a separately extracted distribution package.

A clean working directory was used to verify first-time initialization. The application created its SQLite database and submissions directory in that working directory.

## 5. Limitations

The deployment package has not yet been verified on a separate Windows computer without the development toolchain installed.

The results therefore confirm successful local packaging, extraction, initialization and execution, but do not establish compatibility with every Windows environment.

## 6. Conclusion

The Moderated LMS Windows distribution passed the local deployment verification tests. The application launched successfully from the extracted package, initialized its data in a clean working directory and displayed the first-time administrator setup window.

The Windows distribution is ready for submission as a locally verified deployment artifact.