# Moderated Learning Management System (LMS)
## User Manual — Version 1.0

**Project:** CS4433 Individual Software Engineering Project  
**System:** Moderated Learning Management System  
**Supported roles:** Administrator, Lecturer, Head of Department (HoD), Student

---

## 1. Introduction

The Moderated Learning Management System (LMS) is a desktop application designed to support assignment management, student submissions, marking, moderation and release of results.

The system provides role-based access so that users can only perform actions permitted for their roles. It uses a local SQLite database and can operate without an internet connection.

This manual explains how to use the system through its graphical user interface (GUI).

## 2. Starting the Application

1. Open the folder containing the installed LMS application.
2. Double-click `lms_gui.exe`.
3. If this is the first time the application has been started with a new database, the **First start** window appears.
4. Otherwise, the normal login window appears.

### 2.1 First-Time Administrator Setup

The first user must create an Administrator account.

1. Enter a username.
2. Enter the Administrator's full name.
3. Enter a password containing at least eight characters.
4. Re-enter the password for confirmation.
5. Click **Create**.

The system creates the Administrator account and opens the application.

### 2.2 Logging In

1. Enter your assigned username.
2. Enter your password.
3. Click **Log in**.
4. After successful authentication, the application displays the interface for your role.

If the username or password is incorrect, check the details and try again.

## 3. Administrator User Manual

The Administrator manages the system's users, academic structure, enrolments, audit records and backups.

### 3.1 Managing Users

1. Log in as Administrator.
2. Select the **Users** tab.
3. To register a user, select the appropriate role.
4. Enter the username, full name and initial password.
5. Click **Create user**.
6. Confirm that the new user appears in the users table.

To change a user's full name, select the user, enter the new name and click **Change name**.

To deactivate an account, select the user and click **Deactivate**.

### 3.2 Managing Departments and Courses

1. Open **Departments and courses**.
2. Use the available controls to create or manage departments.
3. Register courses under the appropriate department.
4. Verify that the department and course information is displayed correctly.

### 3.3 Managing Teaching and Enrolment

1. Open **Teaching and enrolment**.
2. Select the relevant course.
3. Assign lecturers to the courses they teach.
4. Enrol students in the appropriate courses.
5. Verify the recorded teaching and enrolment information.

### 3.4 Viewing the Audit Trail

1. Select the **Audit trail** tab.
2. Review the recorded system activities.
3. Use the records to investigate changes where necessary.

Audit records support accountability, especially for changes to marks.

### 3.5 Backing Up System Data

1. Open the **Backup** tab.
2. Choose the available backup operation.
3. Select or enter the backup destination as requested.
4. Confirm the operation.
5. Check that the backup has completed successfully.

A backup should preserve the database and uploaded submission files.

## 4. Lecturer User Manual

The Lecturer manages assignments and assesses student work for assigned courses.

### 4.1 Accessing Courses

1. Log in using a Lecturer account.
2. Open the available teaching or course section.
3. Select a course assigned to you.

### 4.2 Managing Assignments

1. Select the relevant course.
2. Open the assignment management section.
3. Enter the assignment information, including its title, instructions, deadline and maximum mark.
4. Save the assignment.
5. Confirm that it is available in the appropriate course.

### 4.3 Reviewing Student Submissions

1. Open the relevant assignment.
2. View its submitted work.
3. Select a student's submission.
4. Open the latest submitted file where available.
5. Review the student's work and submission details.

The system retains submission information and supports the identification of late work.

### 4.4 Marking and Feedback

1. Select the submission to assess.
2. Enter the raw mark within the permitted range.
3. Provide feedback.
4. Save the marking information.
5. Verify that the submission status reflects the marking action.

Late penalties are calculated according to the system's business rules.

### 4.5 Extensions

Where permitted, a Lecturer may grant an individual student an extension before the submission has been marked.

The system uses the effective deadline when determining lateness.

## 5. Head of Department (HoD) User Manual

The HoD oversees assessment moderation and the release of marks.

### 5.1 Accessing Moderation

1. Log in using a HoD account.
2. Open the moderation section.
3. Select the relevant course or assignment.
4. Review the marked submissions awaiting moderation.

### 5.2 Moderating Marks

1. Select a marked submission.
2. Review the mark and feedback.
3. Perform the moderation action using the available controls.
4. Confirm that the updated status is displayed.

### 5.3 Adjusting Marks

Where an adjustment is necessary:

1. Select the relevant marked submission.
2. Enter the adjusted mark.
3. Provide a written reason for the change.
4. Save the adjustment.
5. Verify that the updated mark is recorded.

The system records mark changes in its audit trail.

### 5.4 Releasing Results

1. Open the relevant moderated results.
2. Review the marks before publication.
3. Use the release action when the results are ready.
4. Confirm that the results have been released.

Students cannot view marks before they are released.

## 6. Student User Manual

Students use the LMS to view assignments, submit work and access released results.

### 6.1 Viewing Assignments

1. Log in using a Student account.
2. Open the student home or course section.
3. Select an enrolled course.
4. View the available assignments.
5. Read the assignment instructions and deadline.

### 6.2 Submitting an Assignment

1. Open the assignment.
2. Select the submission or upload option.
3. Choose the file containing your work.
4. Confirm the submission.
5. Check that the submitted file is shown in the system.

Students should verify that the correct file was submitted.

### 6.3 Resubmitting Work

If the effective deadline has not passed and the submission has not been marked, the student may submit an updated version where the system permits it.

Previous submission versions are retained.

### 6.4 Late Submissions

The LMS applies its configured late-submission rules.

A first submission may be accepted after the deadline, but the final mark can be reduced by a late penalty.

Work submitted more than three started days late receives a final mark of zero, although marking and feedback may still be recorded.

### 6.5 Viewing Feedback and Results

1. Open the relevant assignment.
2. Check the submission status.
3. When results have been released, view the final mark and feedback.

Marks remain hidden from students until release.

## 7. Common Problems and Solutions

| Problem | Recommended action |
|---|---|
| Incorrect login details | Check the username and password and try again. |
| Application does not start | Confirm that the application files and required libraries are present. |
| Assignment is not visible | Check that the correct account and course are being used. |
| Submission cannot be updated | Check whether the deadline has passed or the work has already been marked. |
| Mark is not visible | Confirm that the result has been released. |
| Database or file error | Contact the Administrator and check the installation and backup arrangements. |

## 8. Important Usage Notes

- Users must keep their passwords confidential.
- Each user should work within the permissions assigned to their role.
- Administrators should perform regular backups.
- Lecturers should verify marks and feedback before saving.
- HoDs should provide reasons for mark adjustments.
- Students should submit work