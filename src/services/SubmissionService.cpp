#include "services/SubmissionService.h"

#include "data/Transaction.h"
#include "domain/Errors.h"

namespace lms {

SubmissionService::SubmissionService(Database& db, ISubmissionRepository& submissions,
                                     IAssignmentRepository& assignments,
                                     ICourseRepository& courses, const IPenaltyPolicy& policy,
                                     const IClock& clock, const FileStore& store)
    : db_(db),
      submissions_(submissions),
      assignments_(assignments),
      courses_(courses),
      policy_(policy),
      clock_(clock),
      store_(store) {}

Submission SubmissionService::submit(int studentId, int assignmentId, const std::string& path) {
    store_.validate(path);
    std::optional<Assignment> assignment = assignments_.find(assignmentId);
    if (!assignment) {
        throw ValidationError("No assignment with id " + std::to_string(assignmentId));
    }
    if (!courses_.isEnrolled(assignment->courseId, studentId)) {
        throw AuthError("You are not enrolled in this course");
    }
    const TimePoint deadline =
        assignment->effectiveDeadline(assignments_.findExtension(assignmentId, studentId));
    const TimePoint now = clock_.now();

    std::optional<Submission> existing = submissions_.findCurrent(studentId, assignmentId);
    if (!existing) return firstSubmission(studentId, *assignment, now, deadline, path);

    // BR-06: no resubmission after the deadline or once marked
    if (!existing->isOpen()) {
        throw ValidationError("Resubmission refused: this work has already been marked");
    }
    if (!existing->canResubmit(now, deadline)) {
        throw ValidationError("Resubmission refused: the deadline " + toText(deadline) +
                              " has passed");
    }
    return resubmission(*existing, now, deadline, path);
}

Submission SubmissionService::firstSubmission(int studentId, const Assignment& assignment,
                                              TimePoint now, TimePoint deadline,
                                              const std::string& path) {
    Submission submission;
    submission.assignmentId = assignment.id;
    submission.studentId = studentId;
    submission.setLateness(policy_.daysLate(now, deadline));  // BR-02

    std::string storedPath;
    try {
        Transaction tx(db_);
        submission.id = submissions_.save(submission);
        storedPath = store_.copyIn(path, submission.id, 1);
        submissions_.addVersion(submission.id, SubmissionVersion{1, storedPath, now});
        tx.commit();
    } catch (...) {
        // The Transaction has rolled back the rows, but a rollback can't undo the
        // file copy, so delete it here (Figure 6 note, NFR-07).
        if (!storedPath.empty()) store_.remove(storedPath);
        throw;
    }
    return submission;
}

Submission SubmissionService::resubmission(Submission existing, TimePoint now,
                                           TimePoint deadline, const std::string& path) {
    const int versionNo = submissions_.latestVersion(existing.id)->versionNo + 1;
    // The newest version is the one that will be marked, so its time decides lateness.
    existing.setLateness(policy_.daysLate(now, deadline));

    std::string storedPath;
    try {
        Transaction tx(db_);
        storedPath = store_.copyIn(path, existing.id, versionNo);
        submissions_.addVersion(existing.id, SubmissionVersion{versionNo, storedPath, now});
        submissions_.save(existing);
        tx.commit();
    } catch (...) {
        if (!storedPath.empty()) store_.remove(storedPath);
        throw;
    }
    return existing;
}

std::vector<StudentAssignmentRow> SubmissionService::listForStudent(int studentId) {
    std::vector<StudentAssignmentRow> rows;
    for (const Course& course : courses_.coursesForStudent(studentId)) {
        for (const Assignment& a : assignments_.forCourse(course.id)) {
            const TimePoint deadline =
                a.effectiveDeadline(assignments_.findExtension(a.id, studentId));
            const std::optional<Submission> submission = submissions_.findCurrent(studentId, a.id);
            StudentAssignmentRow row{course.code, a.id, a.title, deadline,
                                     statusForStudent(submission, a.maxMark), ""};
            if (submission && submission->status == SubmissionStatus::Released) {
                row.feedback = submission->feedback;
            }
            rows.push_back(row);
        }
    }
    return rows;
}

int SubmissionService::removeOrphanFiles() {
    return store_.removeOrphans(submissions_.allFilePaths());
}

std::string SubmissionService::statusForStudent(const std::optional<Submission>& submission,
                                                int maxMark) {
    if (!submission) return "Not submitted";
    switch (submission->status) {
        case SubmissionStatus::Submitted:
            return "Submitted";
        case SubmissionStatus::Late:
            return "Late (" + std::to_string(submission->daysLate) +
                   (submission->daysLate == 1 ? " day)" : " days)");
        case SubmissionStatus::Marked:
        case SubmissionStatus::Moderated:
            return "Being marked";  // BR-07: marks stay hidden until released
        case SubmissionStatus::Released: {
            std::string mark = std::to_string(*submission->finalMark);
            mark = mark.substr(0, mark.find('.') + 2);  // one decimal place
            return "Released: " + mark + " / " + std::to_string(maxMark);
        }
    }
    return "Unknown";
}

}  // namespace lms
