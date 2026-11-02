# Moderated Learning Management System (LMS)
## Release Notes — Version 1.0

**Project:** CS4433 Individual Software Engineering Project  
**Release version:** v1.0  
**Release date:** 2 November 2026  
**Platform:** Windows desktop (with CMake-based source builds for supported development environments)

---

## 1. Release Overview

Version 1.0 is the first release of the Moderated Learning Management System (LMS). The application provides a desktop-based platform for managing academic assignments, student submissions, marking, moderation and the release of results.

The system supports four user roles: Administrator, Lecturer, Head of Department (HoD) and Student.

## 2. Main Features

### Administrator
- Create and manage user accounts.
- Manage departments and courses.
- Manage teaching assignments and student enrolments.
- View audit records.
- Back up system data.

### Lecturer
- Manage course assignments and deadlines.
- Review submitted student work.
- Record marks and feedback.
- Manage permitted submission extensions.

### Head of Department
- Review marked submissions.
- Moderate assessment results.
- Adjust marks with a recorded reason.
- Release results to students.

### Student
- View enrolled courses and assignments.
- Upload assignment submissions.
- Resubmit work when permitted.
- View released marks and feedback.

## 3. Technical Features

- C++17 application with a Qt 6 graphical user interface.
- Console application alternative.
- SQLite database for local data storage.
- Role-based access control.
- Password protection using Argon2id.
- Submission file storage and version tracking.
- Automatic late-submission penalty calculation.
- Audit records for mark changes.
- Backup and restore functionality.
- Offline operation.

## 4. Testing and Quality Assurance

The release underwent automated testing and deployment verification.

- **Automated tests:** 214 of 214 passed.
- **Business-layer line coverage:** 97.6% in the recorded coverage run.
- **Static analysis and code review:** Documented during Phase 6.
- **Deployment verification:** Application launched successfully from an extracted Windows distribution.
- **Fresh installation:** First-time Administrator account creation was verified using a new working directory.

## 5. Installation and Deployment

The Windows distribution is packaged as:

`Moderated_LMS_Windows.zip`

The package includes the LMS executables and required runtime libraries.

To install:

1. Extract the ZIP archive.
2. Open the extracted application folder.
3. Run `lms_gui.exe`.
4. Create an Administrator account on first startup.
5. Configure users, departments and courses.

Detailed instructions are available in `INSTALLATION_GUIDE.md`.

## 6. Known Limitations

- The application uses a local SQLite database rather than a central server.
- The current release is intended primarily for offline desktop use.
- Data is not automatically synchronized between separate computers.
- The Windows distribution must be extracted before use.
- Users should maintain regular backups of their data and uploaded files.

## 7. Supporting Documentation

- `DEPLOYMENT_DIAGRAM.puml` — UML deployment diagram source.
- `DEPLOYMENT_DIAGRAM.png` — Rendered deployment diagram.
- `INSTALLATION_GUIDE.md` — Installation instructions.
- `DEPLOYMENT_VERIFICATION.md` — Deployment verification evidence.
- `USER_MANUAL.md` — Instructions for all four user roles.

## 8. Release Status

Version 1.0 is prepared as the initial deployment release of the Moderated LMS.

**Repository:** https://github.com/hataneitumeleng/LMS

**Release tag:** `v1.0` (to be created after final verification).
